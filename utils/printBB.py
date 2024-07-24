import sys

def printBB(num: int):
    for i in range(64):
        end = " " if (i+1) % 8 else "\n"

        print(num>>i&1,end=end)
    
if __name__ == "__main__":
    num = sys.argv[1:]

    printBB(int(num[0]))