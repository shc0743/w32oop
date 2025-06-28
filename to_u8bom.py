import ctypes
from ctypes import wintypes
import os
import codecs

def enable_vt_mode():
    """
    在 Windows 上启用虚拟终端模式
    """
    if not sys.platform.startswith('win'):
        # 如果不是 Windows 系统，直接返回
        return

    # 定义常量
    ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004

    # 获取标准输出句柄
    kernel32 = ctypes.windll.kernel32
    h_out = kernel32.GetStdHandle(-11)  # STD_OUTPUT_HANDLE
    
    # 获取当前控制台模式
    mode = wintypes.DWORD()
    kernel32.GetConsoleMode(h_out, ctypes.byref(mode))
    
    # 启用虚拟终端模式
    mode.value |= ENABLE_VIRTUAL_TERMINAL_PROCESSING
    kernel32.SetConsoleMode(h_out, mode)

def is_utf8_without_bom(filepath):
    try:
        with open(filepath, 'rb') as f:
            content = f.read()
            if content.startswith(codecs.BOM_UTF8):
                return False  # 已经有BOM了
            # 尝试用UTF-8解码
            content.decode('utf-8')
            return True
    except:
        return False

def convert_to_utf8_with_bom(filepath):
    try:
        # 读取原始内容
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 写入带BOM的内容
        with open(filepath, 'w', encoding='utf-8-sig') as f:
            f.write(content)
        
        return True
    except:
        return False

def process_files():
    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith(('.cpp', '.hpp')):
                filepath = os.path.join(root, file)
                if is_utf8_without_bom(filepath):
                    if convert_to_utf8_with_bom(filepath):
                        print(f"\033[32mConverted: {filepath}\033[0m")
                    else:
                        print(f"\033[31mFailed   : {filepath}\033[0m")
                else:
                    print(f"\033[36mSkipped  : {filepath}\033[0m")

if __name__ == '__main__':
    import sys
    if sys.platform.startswith('win'):
        enable_vt_mode()
    process_files()
    input('Enter to continue')