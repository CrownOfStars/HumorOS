from re import findall, fullmatch,search
from tkinter import *

argList = ["info","check",
           "dir","cd","md","rd","newfile","cat","del",
           "copy"]

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


def Func():
    cmdline = input()#TODO: 添加对 含'/' ".." 特殊参数的支持,  
    res = search("[A-Za-z]+",cmdline)
    if res != None:
        if res.group().lower() in argList[0:2]:
            if fullmatch("[\s]*",cmdline[res.span()[1]:]):
                return [(res.group().lower(),)]
            else:
                return [("error","parameters number not match\n")]
        elif res.group().lower() == "dir":#FIXME 带参数dir
            PATH = cmdline[res.span()[1]:].strip()
            if PATH == "":
                return [(res.group().lower(),)]
            elif fullmatch("(/[A-Za-z0-9]+)+",PATH):#绝对路径
                bat = [("cd","/")]
                path = findall("[A-Za-z0-9]+",PATH)
                for item in path:
                    bat.append(("cd",item))
                bat.append((res.group().lower(),))
                return bat
            elif fullmatch("([A-Za-z0-9]+[/])*[A-Za-z0-9]+",PATH):#相对路径
                bat = []
                path = findall("[A-Za-z0-9]+",PATH)
                for item in path:
                    bat.append(("cd",item))
                bat.append((res.group().lower(),))
                return bat
            elif PATH == "-r":
                return [(res.group().lower()+"-r","")]
            else:
                return [("error","parameters number not match\n")]
        elif res.group().lower() == "cd":
            PATH = cmdline[res.span()[1]:].strip()
            if PATH == "/" or PATH == "..":
                return [("cd",PATH)]
            elif fullmatch("(/[A-Za-z0-9]+)+",PATH):#绝对路径
                bat = [("cd","/")]
                path = findall("[A-Za-z0-9]+",PATH)
                for item in path:
                    bat.append(("cd",item))
                return bat
            elif fullmatch("([A-Za-z0-9]+/)*[A-Za-z0-9]+",PATH):#相对路径
                bat = []
                path = findall("[A-Za-z0-9]+",PATH)
                for item in path:
                    bat.append(("cd",item))
                return bat
            else:
                return [("error","invaild parameter\n")]
        elif res.group().lower() in argList[4:6]:
            PATH = cmdline[res.span()[1]:].strip()
            if fullmatch("(/[A-Za-z0-9]+)+",PATH):#绝对路径
                bat = [("cd","/")]
                path = findall("[A-Za-z0-9]+",PATH)
                if len(path[-1]) < 27:
                    for item in path[:-1]:
                        bat.append(("cd",item))
                    bat.append((res.group().lower(),"/"+path[-1]))
                    return bat
                else:
                    return [("error","folder name too long\n")]
            elif fullmatch("([A-Za-z0-9]+[/])*[A-Za-z0-9]+",PATH):#相对路径
                bat = []
                path = findall("[A-Za-z0-9]+",PATH)
                if len(path[-1]) < 27:
                    for item in path[:-1]:
                        bat.append(("cd",item))
                    bat.append((res.group().lower(),"/"+path[-1]))
                    return bat
                else:
                    return [("error","folder name too long\n")]
            else:
                return [("error","invaild parameter\n")]
        elif res.group().lower() in argList[6:9]:
            PATH = cmdline[res.span()[1]:].strip()
            if fullmatch("/([A-Za-z0-9]+/)*[A-Za-z0-9]+(.[A-Za-z0-9]+)?",PATH):#绝对路径
                bat = [("cd","/")]
                path = findall("[A-Za-z0-9]+[.[A-Za-z0-9]+]?",PATH)
                if len(path[-1]) < 28:
                    for item in path[:-1]:
                        bat.append(("cd",item))
                    if res.group().lower() == "newfile":
                        windows = Tk()
                        windows.title("文本编辑——关闭或ctrl+s即提交")
                        windows.geometry("500x300") #界面大小
                        e = Text(windows,height=300,width=300,show=None)
                        e.pack()
                        def on_closing():
                            f = open("cache","w")
                            f.write(e.get("1.0","end")[:-1])
                            f.close()
                            windows.destroy()
                        windows.bind_all('<Control-s>', on_closing)
                        windows.protocol("WM_DELETE_WINDOW", on_closing)
                        windows.mainloop()
                    bat.append((res.group().lower()[:-1],[path[-1]]))
                    return bat
                else:
                    return [("error","file name too long\n")]
            elif fullmatch("([A-Za-z0-9]+/)*[A-Za-z0-9]+(.[A-Za-z0-9]+)?",PATH):#相对路径
                bat = []
                path = findall("[A-Za-z0-9]+[.[A-Za-z0-9]+]?",PATH)
                if len(path[-1]) < 28:
                    for item in path[:-1]:
                        bat.append(("cd",item))
                    if res.group().lower() == "newfile":
                        windows = Tk()
                        windows.title("文本编辑——关闭或ctrl+s即提交")
                        windows.geometry("500x300") #界面大小
                        e = Text(windows,height=300,width=300,show=None)
                        e.pack()
                        def on_closing(arg=None):
                            f = open("cache","w")
                            f.write(e.get("1.0","end")[:-1])
                            f.close()
                            windows.destroy()
                        windows.bind_all('<Control-s>', on_closing)
                        windows.protocol("WM_DELETE_WINDOW", on_closing)
                        windows.mainloop()
                    bat.append((res.group().lower(),path[-1]))
                    return bat
                else:
                    return [("error","file name too long\n")]
            else:
                return [("error","invaild parameter\n")]
        elif res.group().lower() == "copy":    
            restCmd = cmdline[res.span()[1]:].strip()
            if restCmd[:7] == "<host>":
                PATH = ""
                f = open(PATH,"r")
                fAll = f.read()
                f.close()
                f = open("cache","w")
                f.write(fAll)
                #TODO: create file and write
                return ("","","")#XXX
            elif fullmatch("",""):
                #TODO: return filename and path
                return ("","","")#XXX
                pass
            else:
                return [("error","invaild parameter\n")]
        else:    
            return [("error","no such command\n")]
    else:
        return [("error","invaild input\n")]
