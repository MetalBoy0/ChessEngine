import sys


pieces = {
    "E": 0,
    "P": 1,
    "N": 2,
    "B": 3,
    "R": 4,
    "Q": 5,
    "K": 6,
    "p": 9,
    "n": 10,
    "b": 11,
    "r": 12,
    "q": 13,
    "k": 14,
}

# reverse mapping
pieces = {v: k for k, v in pieces.items()}


def decodeMove(move: int):

    fromPos = move & 0x3F
    toPos = (move >> 6) & 0x3F
    promotion = pieces[(move >> 12) & 0x7]
    isPromote = bool((move >> 17) & 0x1)
    isCapture = bool((move >> 18) & 0x1)
    isCastle = bool((move >> 19) & 0x1)
    capturedPiece = pieces[(move >> 20) & 0xF]

    fromPos = (fromPos % 8 + 1, fromPos // 8 + 1)
    toPos = (toPos % 8 + 1, toPos // 8 + 1)
    moveStr = (
        chr(fromPos[0] + 96) + str(fromPos[1]) + chr(toPos[0] + 96) + str(toPos[1])
    )
    print(
        f"Move: {move}\n      {moveStr}\n     From: {fromPos}\n       To: {toPos}\nPromotion: {promotion}\nIsPromote: {isPromote}\nIsCapture: {isCapture}\n IsCastle: {isCastle}\nCapturedPiece: {capturedPiece}"
    )


if __name__ == "__main__":

    args = sys.argv[1:]
    if len(args) != 1:
        print("Usage: python moveDecoder.py <move>")
        sys.exit(1)

    move = int(args[0])

    decodeMove(move)
