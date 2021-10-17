
emptyInode = bytes([0 for _ in range(64)])
emptyByte = b'\x00'
emptyFile = bytes([0 for _ in range(1024)])

def inodeToStr(binary:'bytes'):
    value = "createTime:"+str(int.from_bytes(binary[:8],byteorder='little',signed=False))+"\n"
    value += "mode uid:"+str(int.from_bytes(binary[8:12],byteorder='little',signed=False))+"\n"
    value += "file size:"+str(int.from_bytes(binary[12:16],byteorder='little',signed=False))+"\n"
    value += "base file id:"
    for i in range(10):
        value += str(int.from_bytes(binary[16+i*4:20+i*4],byteorder='little',signed=False))+" "
    return value + "\n\n"

def bitmapToStr(binary:'bytes'):
    return bin(int.from_bytes(binary,byteorder='big',signed=False))

def inodeMapToStr(binary:'bytes'):
    value = "filename:"+binary[:28].decode('utf-8')+"\n"
    value += "inodeid:"+str(int.from_bytes(binary[12:16],byteorder='little',signed=False))+"\n"
    return value

def textfileToStr(binary:'bytes'):
    return binary.decode('utf-8')

k = 1024

def Func():
    f = open("DISK","rb")
####################################
    print("\n-------inode region------\n")
    f.seek(1*k)
    sz = 1024*k
    skep = 64#一个inode有64byte
    for i in range(sz//skep):
        b = f.read(skep)
        if b != emptyInode:
            print(inodeToStr(b))

####################################
    print("\n---file  bitmap region---\n")
    f.seek(1025*k)#filebmregion
    sz = 16*k
    skep = 1
    for i in range(sz//skep):
        b = f.read(skep)
        if b != emptyByte:
            print(str(i)+"\t"+bitmapToStr(b))
####################################
    print("\n---inode bitmap region---\n")
    f.seek(1041*k)
    sz = 2*k
    skep = 1
    for i in range(sz//skep):
        b = f.read(skep)
        if b != emptyByte:
            print(str(i)+"\t"+bitmapToStr(b))
####################################
    print("\n-------file region-------\n")
    # f.seek(1040*k)#fileregion
    # sz = 1000*k
    # skep = k
    # for i in range(sz//skep):
    #     b = f.read(skep)
    #     if b != emptyFile:
    #         print(bitmapToStr(b))
    f.close()
