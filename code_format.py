import os
import subprocess
import re
import sys

def format_all_file(root):
    for root, dirs, files in os.walk(root):

        # root 表示当前正在访问的文件夹路径
        # dirs 表示该文件夹下的子目录名list
        # files 表示该文件夹下的文件list

        # 遍历文件
        for f in files:
            if f.endswith('.c') or f.endswith('.h'):
                full_path = os.path.join(root, f)
                #print("root: %s, path: %s" % (root, full_path))
                print(full_path)
                
                command = 'clang-format -style=file -i %s' % full_path
                #print(command)
                proc = subprocess.run(command, shell=True, stdout=subprocess.PIPE)

if __name__ == '__main__':
    format_all_file('.')

    #proc = subprocess.run(command, shell=True, stdout=subprocess.PIPE)
