#!/usr/bin/python3

# Utility to connect an Entropy player that implements that 2023 Caia protocol
# to my game framework, which uses HTTP POST request and HTTP even streams.
#
# Warning: the code here is pretty ugly!
#
# Example use:
#
# read -r session && ./webclient.py --session "$session" ~/caia/entropy/bin/player1
# http://localhost:8080/session.html#sessionId=XXXXXX&playerKeys=XXXXXX

import argparse
import json
import re
import socket
import subprocess
import sys
from urllib.parse import urlparse, urljoin, quote, unquote
from urllib.request import urlopen, Request
from urllib.error import HTTPError

argument_parser = argparse.ArgumentParser(description='Connects an Entropy player using the CodeCup 2021 protocol to the GameFrame server')
argument_parser.add_argument('--session', type=str, nargs=1, help='Session URL', required=True)
argument_parser.add_argument('command', type=str, nargs=1, help='Path to player executable')
argument_parser.add_argument('arg', type=str, nargs='*', help='Player arguments')

GAME_ID = 'entropy'
SESSION_URL_TEMPLATE = 'api/sessions/{sessionId}?playerKeys={playerKey}'
EVENT_STREAM_SUFFIX = '&format=event-stream'
MOVE_PATTERN = re.compile("^Start|[1-7]?[A-G][a-g]|[A-G][a-g][A-G][a-g]$")


def ReadEventStream(f):
    data = b''
    while True:
        line = f.readline()
        if not line:
            break
        if line.startswith(b'data:'):
            if len(line) > 5 and line[5] == b' ':
                data += line[6:]
            else:
                data += line[5:]
        elif not line.strip():
            if data:
                yield data.decode('utf-8')
                data = b''


def FieldToCaia(i):
    return chr(ord('A') + i // 7) + chr(ord('a') + i % 7)


def FieldFromCaia(s):
    assert len(s) == 2
    r = ord(s[0]) - ord('A')
    c = ord(s[1]) - ord('a')
    assert 0 <= r < 7
    assert 0 <= c < 7
    return 7*r + c


def ChaosMoveFromCaia(s):
    assert len(s) == 2
    return FieldFromCaia(s)


def OrderMoveFromCaia(s):
    assert len(s) == 4
    i = FieldFromCaia(s[0:2])
    j = FieldFromCaia(s[2:4])
    if i == j:
        return []
    return [i, j]


def PlayGame(command_args, session_url, event_stream_url, player_id, player_key):
    popen = subprocess.Popen(args=command_args, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    history = []
    turn_index = 0
    for update in ReadEventStream(urlopen(event_stream_url)):
        update = json.loads(update)
        if update['gameId'] != GAME_ID:
            print('Update has incorrect gameId!')
            sys.exit(1)
        state = update['state']
        if state['size'] != 7:
            print('Game size incorrect (only size 7 is supported)')
            sys.exit(1)
        pieces = state['pieces']
        last_move = state['lastMove']
        next_player = state['nextPlayer']
        next_piece = state['nextPiece']
        last_move_string = None
        if next_player == 'Chaos':
            if next_piece == 0:
                # Wait until _random has picked the next piece.
                # This case shouldn't normally happen since _random auto-plays
                # automatically.
                continue
            if len(last_move) == 0:
                # Synthesize a dummy no-op move
                for i, v in enumerate(state['pieces']):
                    if v != 0:
                        last_move_string = FieldToCaia(i) * 2
                        break
                else:
                    last_move_string = 'Start'
            else:
                i, j = last_move
                last_move_string = FieldToCaia(i) + FieldToCaia(j)
        else:
            assert next_player == 'Order' or next_player is None
            color = pieces[last_move]
            assert color != 0
            last_move_string = str(color) + FieldToCaia(last_move)

        assert last_move_string is not None
        if last_move_string != 'Start':
            history.append(last_move_string)

        if next_player is None:
            # End of game.
            try:
              popen.stdin.write(b'Quit\n')
              popen.stdin.close()
            except:
              # Ignore errors here. It's possible the player already quit.
              pass
            popen.wait()
            return history

        if next_player == player_id:
            print('Sent', last_move_string)
            assert MOVE_PATTERN.match(last_move_string)
            popen.stdin.write((last_move_string + '\n').encode('utf-8'))
            if next_player == 'Chaos':
                popen.stdin.write((str(next_piece) + '\n').encode('utf-8'))
            popen.stdin.flush()

            next_move_string = popen.stdout.readline().decode('utf-8').strip()
            print('Received', next_move_string)
            assert MOVE_PATTERN.match(next_move_string)
            next_move = None
            if next_player == 'Chaos':
                next_move = ChaosMoveFromCaia(next_move_string)
            if next_player == 'Order':
                next_move = OrderMoveFromCaia(next_move_string)
            update = {
                'gameVersion': update['gameVersion'],
                'moveCount': update['moveCount'],
                'playerKey': player_key,
                'player': player_id,
                'move': next_move
            }
            request = Request(session_url)
            request.add_header('Content-Type', 'application/json; charset=utf-8')
            try:
                urlopen(request, json.dumps(update).encode('utf-8'))
            except HTTPError as e:
                print(e)
                print(e.read().decode('utf-8'))
                sys.exit(1)


def Main():
    args = argument_parser.parse_args()
    fragment = urlparse(args.session[0]).fragment
    if not fragment:
        print('Session URL is missing a URL fragment!')
        sys.exit(1)
    params = {}
    for part in fragment.split('&'):
        if '=' in part:
            key, value = map(unquote, part.split('=', 1))
            params[key] = value
    for key in ('sessionId', 'playerKeys'):
        if key not in params:
            print('Session URL is missing required parameter [{}]'.format(key))
            sys.exit(1)
    session_id = params['sessionId']
    player_keys = params['playerKeys']
    if ',' in player_keys or player_keys.count(':') != 1:
        print('Exactly 1 player key is required!')
        sys.exit(1)
    player_id, player_key = params['playerKeys'].split(':')
    if player_id not in ('Chaos', 'Order'):
        print('playerId must be [Chaos] or [Order]')
        sys.exit(1)

    session_url = urljoin(
        args.session[0],
        SESSION_URL_TEMPLATE.format(
            playerKey=quote(player_key),
            sessionId=quote(session_id)))
    event_stream_url = session_url + EVENT_STREAM_SUFFIX

    history = PlayGame(args.command + args.arg, session_url, event_stream_url, player_id, player_key)
    print('Transcript:', ','.join(history))


if __name__ == '__main__':
    Main()
