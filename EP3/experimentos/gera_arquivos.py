import sys
import random as rd
import string as st


def gera_arq(nome, tam):
    with open(nome, "a") as file:
        for i in range(int(tam)):
            file.write(rd.choice(st.ascii_letters))


if __name__ == "__main__":
    gera_arq(sys.argv[1], sys.argv[2])
