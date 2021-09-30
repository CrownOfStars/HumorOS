while True:
    n = int(input(">"))
    f = open("DISK","rb")
    sz = int(input(">size"))
    f.seek(n)
    b = f.read(sz)
    print(b)
    f.close()