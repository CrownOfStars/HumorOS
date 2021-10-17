from re import findall, fullmatch
from os.path import isfile
from tkinter import *

"""
针对路径名作出如下解析方案:
a/b/c/d==>
cd a
cd b
cd d

cd a/b/c/d =>
[("cd","a"),("cd","b"),("cd","c"),("cd","d")]

md a/b/c/d =>
[("cd","a"),("cd","b"),("cd","c"),("md","d")]

"""

def InfoDFA(cmd:'str'):
    if cmd[:3].lower() == "fod" and cmd[3].isspace():
        path = cmd[3:].strip()
        bat = []
        if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",path):
            bat.append(("cd","/"))
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append("info","/"+paths[-1])
                return bat
            else:
                return [("error","file name too long\n")]
            
        elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+path):
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("info","/"+paths[-1]))
                return bat
            else:
                return [("error","file name too long\n")]
        else:
            return [("error","invaild parameter\n")]

    elif cmd[:3].lower() == "fof" and cmd[3].isspace():
        path = cmd[3:].strip()
        bat = []
        if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",path):
            bat.append(("cd","/"))
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append("info",paths[-1])
                return bat
            else:
                return [("error","file name too long\n")]
            
        elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+path):
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("info",paths[-1]))
                return bat
            else:
                return [("error","file name too long\n")]
        else:
            return [("error","invaild parameter\n")]

    else:
        return [("error","no such command\n")]

def CdDFA(cmd:'str'):
    if len(cmd) > 0 and cmd[0].isspace():
        path = cmd.strip()
        bat = []
        if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",path):
            bat.append(("cd","/"))
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            return bat
        elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+path):
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            return bat
        elif path == "..":
            return [("cd","..")]
        
        elif path == "/":
            return [("cd","/")]
            
        else:
            return [("error","invaild parameter\n")]

def MdDFA(cmd:'str'):
    if len(cmd) > 0 and cmd[0].isspace():
        path = cmd.strip()
        bat = []
        if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",path):
            bat.append(("cd","/"))
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("md","/"+paths[i]))
                return bat
            else:
                return [("error","file name too long\n")]

        elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+path):
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("md","/"+paths[-1]))
                return bat
            else:
                return [("error","file name too long\n")]
        else:
            return [("error","invaild parameter\n")]
    else:
        return [("error","no such command\n")]

def DirDFA(cmd:'str'):
    if cmd[0] == "r" and len(cmd) == 1:
        return [("dir",)]
    elif cmd[0].lower() == "r" and cmd[1].isspace():
        path = cmd[1:].strip()
        if path == "-r":
            return [("dir-r",)]
        bat = []
        if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",path):
            bat.append(("cd","/"))
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            bat.append(("dir",))
            return bat
        elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+path):
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            bat.append(("dir",))
            return bat
        else:
            return [("error","invaild parameter\n")]

    else:
        return [("error","no such command\n")]

def CopyDFA(cmd:'str'):
    #禁止绝对路径->相对论路径
    if cmd[:2] == "py" and cmd[2].isspace():
        twopath = cmd[2:].strip()
        if twopath[:6] == "<host>":
            blank = -1
            paths = twopath[6:].strip()
            for i in range(len(paths)):
                if paths[i].isspace():
                    blank = i
                    break
            if blank == -1:
                return [("error","invaild parameter\n")]
            else:
                if isfile(paths[:blank]):
                    f = open(paths[:blank],"rb")
                    content = f.read()
                    f.close()
                    path = paths[blank:].strip()
                    bat = []
                    if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",path):
                        bat.append(("cd","/"))
                        paths = findall("[A-Za-z0-9\.]+",path)
                        for i in range(len(paths)-1):
                            if len(paths[i]) < 27:
                                bat.append(("cd","/"+paths[i]))
                            else:
                                return [("error","file name too long\n")]

                        if  len(paths[-1]) < 27:
                            fw = open('cache','wb')
                            fw.write(content)
                            fw.close()
                            bat.append(("newfile",paths[-1]))
                            return bat
                        else:
                            return [("error","file name too long\n")]

                    elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+path):
                        paths = findall("[A-Za-z0-9\.]+",path)
                        for i in range(len(paths)-1):
                            if len(paths[i]) < 27:
                                bat.append(("cd","/"+paths[i]))
                            else:
                                return [("error","file name too long\n")]
                        if  len(paths[-1]) < 27:
                            fw = open('cache','wb')
                            fw.write(content)
                            fw.close()
                            bat.append(("newfile",paths[-1]))
                            return bat
                        else:
                            return [("error","file name too long\n")]
                    else:
                        return [("error","invaild parameter\n")]

        else:#非本地文件
            blank = -1
            for i in range(len(twopath)):
                if twopath[i].isspace():
                    blank = i
                    break
            Srcpath = twopath[:blank]
            Despath = twopath[blank:].strip()
            bat = []
            if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",Srcpath):
                bat.append(("cd","/"))
                paths = findall("[A-Za-z0-9\.]+",Srcpath)
                for i in range(len(paths)-1):
                    if len(paths[i]) < 27:
                        bat.append(("cd","/"+paths[i]))
                    else:
                        return [("error","file name too long\n")]
                if len(paths[-1]) < 27:
                    bat.append(("copy",paths[-1]))
                    if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",Despath):
                        bat.append(("cd","/"))
                        paths = findall("[A-Za-z0-9\.]+",Despath)
                        for i in range(len(paths)-1):
                            if len(paths[i]) < 27:
                                bat.append(("cd","/"+paths[i]))
                            else:
                                return [("error","file name too long\n")]
                        if len(paths[-1]) < 27:
                            bat.append(("newfile",paths[-1]))
                            return bat
                        else:
                            return [("error","file name too long\n")]
                    else:
                        return [("error","invaild parameter\n")]
                else:
                    return [("error","file name too long\n")]
            elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+Srcpath):
                paths = findall("[A-Za-z0-9\.]+",Srcpath)
                for i in range(len(paths)-1):
                    if len(paths[i]) < 27:
                        bat.append(("cd","/"+paths[i]))
                    else:
                        return [("error","file name too long\n")]
                if len(paths[-1]) < 27:
                    bat.append(("copy",paths[-1]))
                    if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",Despath):
                        bat.append(("cd","/"))
                        paths = findall("[A-Za-z0-9\.]+",Despath)
                        for i in range(len(paths)-1):
                            if len(paths[i]) < 27:
                                bat.append(("cd","/"+paths[i]))
                            else:
                                return [("error","file name too long\n")]
                        if len(paths[-1]) < 27:
                            bat.append(("newfile",paths[-1]))
                            return bat
                        else:
                            return [("error","file name too long\n")]

                    elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+Despath):
                        for i in range(len(bat)-1):
                            bat.append(("cd",".."))
                        paths = findall("[A-Za-z0-9\.]+",Despath)
                        for i in range(len(paths)-1):
                            if len(paths[i]) < 27:
                                bat.append(("cd","/"+paths[i]))
                            else:
                                return [("error","file name too long\n")]
                        if len(paths[-1]) < 27:
                            bat.append(("newfile",paths[-1]))
                            return bat
                        else:
                            return [("error","file name too long\n")]
                    
                    else:
                        return [("error","invaild parameter\n")]
            else:
                return [("error","invaild parameter\n")]

    else:
        return [("error","no such command\n")]

def NewfileDFA(cmd:'str'):
      if cmd[:5] == "wfile" and cmd[5].isspace():
        path = cmd[5:].strip()
        bat = []
        if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",path):
            bat.append(("cd","/"))
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("newfile",paths[-1]))
                windows = Tk()
                windows.title("文本编辑——ctrl+s提交")
                windows.geometry("500x300") #界面大小
                e = Text(windows,height=300,width=300,show=None)
                e.pack()
                def on_closing(a):
                    f = open("cache","w")
                    f.write(e.get("1.0","end")[:-1])
                    f.close()
                    windows.destroy()
                windows.bind_all('<Control-s>', on_closing)
                windows.mainloop()
                return bat
            else:
                return [("error","file name too long\n")]

        elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+path):
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("newfile",paths[-1]))
                windows = Tk()
                windows.title("文本编辑——ctrl+s提交")
                windows.geometry("500x300") #界面大小
                e = Text(windows,height=300,width=300,show=None)
                e.pack()
                def on_closing(a):
                    f = open("cache","w")
                    f.write(e.get("1.0","end")[:-1])
                    f.close()
                    windows.destroy()
                windows.bind_all('<Control-s>', on_closing)
                windows.mainloop()
                return bat
            else:
                return [("error","file name too long\n")]

        else:
            return [("error","invaild parameter\n")]

def CatDFA(cmd:'str'):
    if cmd[0] == "t" and cmd[1].isspace():
        path = cmd[1:].strip()
        bat = []
        if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",path):
            bat.append(("cd","/"))
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("cat",paths[-1]))
                return bat
            else:
                return [("error","file name too long\n")]

        elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+path):
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("cat",paths[-1]))
                return bat
            else:
                return [("error","file name too long\n")]

        else:
            return [("error","invaild parameter\n")]

def DelDFA(cmd:'str'):
    
    if cmd[0] == "l" and cmd[1].isspace():
        path = cmd[1:].strip()
        bat = []
        if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",path):
            bat.append(("cd","/"))
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("del",paths[-1]))
                return bat
            else:
                return [("error","file name too long\n")]

        elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+path):
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("del",paths[-1]))
                return bat
            else:
                return [("error","file name too long\n")]
        else:
            return [("error","invaild parameter\n")]

    else:
        return [("error","no such command")]

def RdDFA(cmd:'str'):
    if len(cmd) > 0 and cmd[0].isspace():
        path = cmd.strip()
        bat = []
        if fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+",path):
            bat.append(("cd","/"))
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append("rd","/"+paths[-1])
                return bat
            else:
                return [("error","file name too long\n")]

        elif fullmatch("(/[A-Za-z0-9]+[\.]?[A-Za-z0-9]*)+","/"+path):
            paths = findall("[A-Za-z0-9\.]+",path)
            for i in range(len(paths)-1):
                if len(paths[i]) < 27:
                    bat.append(("cd","/"+paths[i]))
                else:
                    return [("error","file name too long\n")]
            if len(paths[-1]) < 27:
                bat.append(("rd","/"+paths[-1]))
                return bat
            else:
                return [("error","file name too long\n")]

        else:
            return [("error","invaild parameter\n")]

    else:
        return [("error","no such command")]

def CheckDFA(cmd:'str'):
    if cmd[:3].lower() == "eck" and (cmd[3:].isspace() or cmd[3:] == ""):
        return [("check",)]
    else:
        return [("error","no such command\n")]

switch = {
    "in":InfoDFA,
    "cd":CdDFA,
    "md":MdDFA,
    "di":DirDFA,
    "co":CopyDFA,
    "ne":NewfileDFA,
    "ca":CatDFA,
    "de":DelDFA,
    "rd":RdDFA,
    "ch":CheckDFA
    }

def default(cmd):
    return [("error","no such command\n")]

def xFunc():
    cmdline = input().strip()
    return switch.get(cmdline[:2].lower(),default)(cmdline[2:])

