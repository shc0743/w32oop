import os, sys
import subprocess
import shutil
import argparse
import time
from pathlib import Path
import ctypes
from ctypes import wintypes
import hashlib

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

def find_cpp_files(base_dir, exclude_prefixes=('.',)):
    """查找所有符合条件的 .cpp 文件"""
    cpp_files = []
    for root, _, files in os.walk(base_dir):
        # 跳过以点开头的目录
        if any(root.startswith(os.path.join(base_dir, prefix)) for prefix in exclude_prefixes):
            continue

        for file in files:
            if file.endswith('.cpp'):
                full_path = os.path.join(root, file)
                cpp_files.append(full_path)

    return cpp_files

def compile_to_objs(cpp_files, output_dir, include_dirs, cl_exe='cl.exe'):
    """编译所有 .cpp 文件为 .obj 文件"""
    obj_files = []
    os.makedirs(output_dir, exist_ok=True)
    compiled_count = 0
    skipped_count = 0

    for cpp_file in cpp_files:
        # 计算输出路径：.cache/build/相对路径/文件名.obj
        rel_path = os.path.relpath(cpp_file, os.getcwd())
        hash_path = hashlib.sha256(rel_path.encode()).hexdigest()
        # 取文件名（去掉扩展名）
        file_name = os.path.splitext(os.path.basename(cpp_file))[0]
        obj_path = os.path.join(output_dir, f'{file_name}@{hash_path}.obj')

        obj_dir = os.path.dirname(obj_path)
        os.makedirs(obj_dir, exist_ok=True)

        # 检查是否需要重新编译
        need_compile = True
        print(f"\n\033[4m\033[96m{rel_path} --> {file_name}@{hash_path}.obj\033[0m", end='')
        if os.path.exists(obj_path):
            src_mtime = os.path.getmtime(cpp_file)
            obj_mtime = os.path.getmtime(obj_path)
            if src_mtime < obj_mtime:
                # 源文件比目标文件旧，跳过编译
                need_compile = False
                skipped_count += 1
                print(f" \033[93mNot Modified\033[0m")

        if need_compile:
            # 构建编译命令
            cmd = [
                cl_exe,
                '/c',  # 只编译不链接
                '/EHsc', '/Zi', '/MT', '/std:c++20', '/D_UNICODE', '/DUNICODE',
                '/nologo',
                f'/Fo{obj_path}',
                f'{cpp_file}'
            ]

            # 添加包含目录
            for include_dir in include_dirs:
                cmd.append(f'/I"{include_dir}"')

            print('')

            # 执行编译
            result = subprocess.call(cmd)

            if result != 0:
                print(f"编译错误: {cpp_file}")
                return False
            else:
                compiled_count += 1

        obj_files.append(obj_path)

    print(f"\n\033[34m编译统计: {compiled_count} 个文件重新编译, {skipped_count} 个文件跳过\033[0m")
    return True

sys_lib = [
    'kernel32.lib',
    'user32.lib',
    'gdi32.lib',
    'comdlg32.lib',
    'advapi32.lib',
    'shell32.lib',
    'ole32.lib',
    'oleaut32.lib',
    'uuid.lib',
    'odbc32.lib',
    'odbccp32.lib',
]

def build_examples(examples_dir, obj_dir, link_exe='link.exe'):
    """构建所有示例程序"""
    if not os.path.exists(examples_dir):
        print(f"示例目录不存在: {examples_dir}")
        return True

    success = True
    example_count = 0

    # 遍历所有示例目录
    for root, dirs, files in os.walk(examples_dir):
        # 只处理包含 demo.cpp 的目录
        if 'demo.cpp' in files:
            demo_cpp = os.path.join(root, 'demo.cpp')
            example_name = os.path.basename(root)
            exe_output = os.path.join(root, f"{example_name}.exe")
            
            # 为每个示例创建独立的obj目录
            example_obj_dir = os.path.join(obj_dir, '..', "examples", example_name)
            os.makedirs(example_obj_dir, exist_ok=True)

            print(f"\n\033[4m\033[96m{example_name}\033[0m")

            # 编译 demo.cpp
            demo_obj = os.path.join(example_obj_dir, "demo.obj")

            compile_cmd = [
                'cl.exe',
                '/c', '/EHsc', '/Zi', '/MT', '/std:c++20',
                '/nologo',
                '/D_UNICODE', '/DUNICODE',
                f'/Fo{demo_obj}',
                f'{demo_cpp}'
            ]
            print(f'\033[94m{' '.join(compile_cmd)}\033[0m')

            # 添加包含目录 (包括项目根目录)
            root_dir = os.getcwd()
            compile_cmd.append(f'/I"{root_dir}"')

            # 执行编译
            compile_result = subprocess.call(compile_cmd)
            if compile_result != 0:
                print(f"\033[31m编译 demo.cpp 失败: {demo_cpp}\033[0m")
                success = False
                continue

            # 只链接库的obj文件和当前示例的demo.obj
            all_obj_files = []
            # 添加库的obj文件
            for root_obj, _, obj_files in os.walk(os.path.join(obj_dir)):
                for obj_file in obj_files:
                    if obj_file.endswith('.obj'):
                        all_obj_files.append(os.path.join(root_obj, obj_file))
            # 添加当前示例的demo.obj
            all_obj_files.append(demo_obj)

            # 构建链接命令
            link_cmd = [
                link_exe,
                '/OUT:' + exe_output,
                '/MACHINE:X64',
                '/MANIFEST:EMBED',
                '/nologo',
                '/INCREMENTAL:NO',
                '/MANIFESTUAC:level="asInvoker" uiAccess="false"',
                '/OPT:REF'
            ] + all_obj_files + sys_lib

            # 执行链接
            print(f"\033[94m{' '.join(link_cmd)}\033[0m")
            link_result = subprocess.call(link_cmd)

            if link_result != 0:
                print(f"\033[31m链接失败: {example_name} ({link_result})\033[0m")
                success = False
            else:
                print(f"\033[92m成功生成: {exe_output}\033[0m")
                example_count += 1

    print(f"\n示例构建完成: 成功 {example_count} 个")
    return success

def main():
    parser = argparse.ArgumentParser(description='w32oop 项目构建系统')
    parser.add_argument('--clean', action='store_true', help='清理构建缓存')
    args = parser.parse_args()

    # 设置路径
    project_root = os.getcwd()
    cache_dir = os.path.join(project_root, '.cache')
    obj_dir = os.path.join(cache_dir, 'build')

    # 清理构建缓存
    if args.clean:
        if os.path.exists(cache_dir):
            shutil.rmtree(cache_dir)
            print(f"已清理构建缓存: {cache_dir}")
        return

    start_time = time.time()

    # 步骤 1: 找到所有 .cpp 文件 (排除点开头的目录)
    print("\033[94m扫描源文件...\033[0m")
    cpp_files = find_cpp_files(project_root, exclude_prefixes=('.',))
    print(f"\033[96m找到 {len(cpp_files)} 个 .cpp 文件\033[0m")

    # 步骤 2: 编译为中间文件
    print("\n\033[34m编译库文件...\033[0m")
    include_dirs = [project_root]  # 添加项目根目录作为包含路径
    if not compile_to_objs(cpp_files, obj_dir, include_dirs):
        print("\033[31m编译库文件失败，终止构建\033[0m")
        sys.exit(1)

    # 步骤 3: 构建示例
    print("\n\033[34m构建示例程序...\033[0m")
    examples_dir = os.path.join(project_root, '.examples')
    success_count = build_examples(examples_dir, obj_dir)

    # 统计构建时间
    elapsed = time.time() - start_time
    print(f"\n\033[32m构建完成! 耗时: {elapsed:.2f} 秒\033[0m")

    if success_count == 0:
        print("\033[31m构建示例程序失败\033[0m")
        sys.exit(1)
    else:
        print("\033[92m构建示例程序成功\033[0m")

if __name__ == "__main__":
    enable_vt_mode()
    try:
        main()
    except KeyboardInterrupt:
        print("\n\033[91m构建被中断!\033[0m")
