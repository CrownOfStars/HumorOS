from re import findall, match

argLen = {
    "info":0,
    "cd":1,
    "dir":0,
    "md":1,
    "rd":1,
    "newfile":1,
    "cat":1,
    "copy":2,
    "del":1,
    "check":0
}

argList = ["info","cd","dir","md","rd","newfile","cat","copy","del","check"]

def Func():
    cmd = input()
    if match("[a-z]+([\s]+[A-Za-z0-9]+)*",cmd):
        cmds = findall("[A-Za-z0-9]+",cmd)
        if cmds[0] not in argList:
            return "error", ["no such command\n"]
        elif len(cmds) == argLen.get(cmds[0]) + 1:
            for arg in cmds[1:]:
                if len(arg) > 27:
                    return "error",["parameter too long\n"]
            return cmds[0],cmds[1:]
        else:
            return "error",["parameters number not match\n"]
    else:
        return "error",["invaild input\n"]
