import os, sys
import subprocess
import shutil
import argparse
import time
from pathlib import Path
import ctypes
from ctypes import wintypes
import hashlib
import locale
import threading
from concurrent.futures import ThreadPoolExecutor, as_completed

_OUTPUT_LOCK = threading.Lock()
DEFAULT_JOBS = os.cpu_count() or 1

def write_output(text, stream=None):
    if stream is None:
        stream = sys.stdout
    with _OUTPUT_LOCK:
        stream.write(text)
        stream.flush()

def get_output_encoding(stream):
    if sys.platform.startswith('win'):
        code_page = ctypes.windll.kernel32.GetConsoleOutputCP()
        if code_page:
            return f'cp{code_page}'
    return getattr(stream, 'encoding', None) or locale.getpreferredencoding(False)

def run_build_command(cmd, task_number):
    process = subprocess.Popen(
        cmd,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    def forward_lines(pipe, stream):
        encoding = get_output_encoding(stream)
        for raw_line in iter(pipe.readline, b''):
            line = raw_line.decode(encoding, errors='replace')
            write_output(f'{task_number}> {line}', stream)
        pipe.close()

    readers = [
        threading.Thread(target=forward_lines, args=(process.stdout, sys.stdout)),
        threading.Thread(target=forward_lines, args=(process.stderr, sys.stderr)),
    ]
    for reader in readers:
        reader.start()

    result = process.wait()
    for reader in readers:
        reader.join()
    return result

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

COMPILE_COMMON = [
    '/c',
    '/EHsc', '/permissive-', '/Z7', '/MT', '/std:c++20', '/D_UNICODE', '/DUNICODE', '/DNDEBUG',
    '/nologo',
    '/GS', '/GL', '/W4', '/WX', '/Gy', '/Zc:wchar_t', '/Zc:inline', '/fp:precise',
    '/Qspectre', '/QIntel-jcc-erratum', '/Gm-', '/O2', '/Gd', '/Oi', '/guard:ehcont', '/guard:cf',
]

def make_compile_cmd(src, obj, include_dirs, cl_exe='cl.exe', extra_flags=()):
    cmd = [cl_exe, *COMPILE_COMMON, *extra_flags]
    for d in include_dirs:
        cmd.append(f'/I{d}')
    cmd.append(f'/Fo{obj}')
    cmd.append(src)
    return cmd

def compile_to_objs(cpp_files, output_dir, include_dirs, cl_exe='cl.exe', max_workers=None):
    """编译所有 .cpp 文件为 .obj 文件"""
    if max_workers is None:
        max_workers = DEFAULT_JOBS

    os.makedirs(output_dir, exist_ok=True)
    compile_tasks = []
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

        if os.path.exists(obj_path):
            src_mtime = os.path.getmtime(cpp_file)
            obj_mtime = os.path.getmtime(obj_path)
            if src_mtime < obj_mtime:
                skipped_count += 1
                write_output(f"\033[4m\033[96m{rel_path}\033[0m \033[93mNot Modified\033[0m\n")
                continue

        task_number = len(compile_tasks) + 1
        write_output(f"\n\033[4m\033[96m{task_number}> {rel_path} --> {file_name}@{hash_path}.obj\033[0m\n")
        cmd = make_compile_cmd(cpp_file, obj_path, include_dirs, cl_exe)

        # Keep the existing quoted include argument for compatibility with the current script.
        for include_dir in include_dirs:
            cmd.append(f'/I"{include_dir}"')

        compile_tasks.append((task_number, cpp_file, cmd))

    def compile_one(task):
        task_number, cpp_file, cmd = task
        try:
            result = run_build_command(cmd, task_number)
        except OSError as error:
            write_output(f"{task_number}> 编译器启动失败: {error}\n")
            return False

        if result != 0:
            write_output(f"{task_number}> 编译错误: {cpp_file} ({result})\n")
            return False
        return True

    success = True
    compiled_count = 0
    if compile_tasks:
        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            futures = [executor.submit(compile_one, task) for task in compile_tasks]
            for future in as_completed(futures):
                task_success = future.result()
                success = task_success and success
                compiled_count += task_success

    write_output(f"\n\033[34m编译统计: {compiled_count} 个文件重新编译, {skipped_count} 个文件跳过\033[0m\n")
    return success

sys_lib = [
    'kernel32.lib',
    'user32.lib',
    'gdi32.lib',
    'comdlg32.lib',
    'comctl32.lib',
    'advapi32.lib',
    'shell32.lib',
    'ole32.lib',
    'oleaut32.lib',
    'uuid.lib',
    'odbc32.lib',
    'odbccp32.lib',
]

def build_examples(examples_dir, obj_dir, link_exe='link.exe', max_workers=None):
    """构建所有示例程序"""
    if not os.path.exists(examples_dir):
        print(f"示例目录不存在: {examples_dir}")
        return True
    if max_workers is None:
        max_workers = DEFAULT_JOBS

    examples = []
    for root, dirs, files in os.walk(examples_dir):
        if 'demo.cpp' in files and ('deprecated' not in root):
            examples.append((root, os.path.join(root, 'demo.cpp')))

    library_obj_files = []
    for root_obj, _, obj_files in os.walk(obj_dir):
        library_obj_files.extend(
            os.path.join(root_obj, obj_file)
            for obj_file in obj_files
            if obj_file.endswith('.obj')
        )

    root_dir = os.getcwd()

    def build_one(task):
        task_number, root, demo_cpp = task
        example_name = os.path.basename(root)
        exe_output = os.path.join(root, f"{example_name}.exe")
        example_obj_dir = os.path.join(obj_dir, '..', "examples", example_name)
        os.makedirs(example_obj_dir, exist_ok=True)

        write_output(f"\n\033[4m\033[96m{task_number}> {example_name}\033[0m\n")
        demo_obj = os.path.join(example_obj_dir, "demo.obj")
        compile_cmd = make_compile_cmd(demo_cpp, demo_obj, include_dirs=[root_dir], extra_flags=())
        compile_cmd.append(f'/I"{root_dir}"')
        write_output(f"\033[94m{task_number}> {' '.join(compile_cmd)}\033[0m\n")

        try:
            compile_result = run_build_command(compile_cmd, task_number)
        except OSError as error:
            write_output(f"{task_number}> 编译器启动失败: {error}\n")
            return False
        if compile_result != 0:
            write_output(f"\033[31m{task_number}> 编译 demo.cpp 失败: {demo_cpp} ({compile_result})\033[0m\n")
            return False

        link_cmd = [
            link_exe,
            '/OUT:' + exe_output,
            '/MACHINE:X64',
            '/MANIFEST:EMBED',
            f'/MANIFESTINPUT:"{root_dir}/.examples/input.manifest"',
            '/nologo',
            '/INCREMENTAL:NO',
            '/MANIFESTUAC:level="asInvoker" uiAccess="false"',
            '/NXCOMPAT',
            '/DYNAMICBASE',
            '/OPT:REF',
            '/OPT:ICF',
            '/LTCG',
            '/guard:ehcont', '/guard:cf',
            '/RELEASE', '/Brepro',
        ] + library_obj_files + [demo_obj] + sys_lib

        write_output(f"\033[94m{task_number}> {' '.join(link_cmd)}\033[0m\n")
        try:
            link_result = run_build_command(link_cmd, task_number)
        except OSError as error:
            write_output(f"{task_number}> 链接器启动失败: {error}\n")
            return False

        if link_result != 0:
            write_output(f"\033[31m{task_number}> 链接失败: {example_name} ({link_result})\033[0m\n")
            return False

        write_output(f"\033[92m{task_number}> 成功生成: {exe_output}\033[0m\n")
        return True

    tasks = [
        (task_number, root, demo_cpp)
        for task_number, (root, demo_cpp) in enumerate(examples, start=1)
    ]
    example_count = 0
    if tasks:
        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            futures = [executor.submit(build_one, task) for task in tasks]
            for future in as_completed(futures):
                example_count += future.result()

    success = example_count == len(examples)
    write_output(f"\n示例构建完成: 成功 {example_count} 个\n")
    return success

def main():
    parser = argparse.ArgumentParser(description='w32oop 项目构建系统')
    parser.add_argument('--clean', action='store_true', help='清理构建缓存')
    parser.add_argument('--jobs', type=int, default=DEFAULT_JOBS, help='最大并发构建任务数')
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error('--jobs 必须是大于 0 的整数')

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
    cpp_files = find_cpp_files(project_root, exclude_prefixes=('.', 'build'))
    print(f"\033[96m找到 {len(cpp_files)} 个 .cpp 文件\033[0m")

    # 步骤 2: 编译为中间文件
    print("\n\033[34m编译库文件...\033[0m")
    print('* 注: 已知问题: 目前只能检测到 .cpp 文件的修改，无法检测 .h 及其他依赖文件的修改，请手动清理缓存以完全重新编译')
    include_dirs = [project_root]  # 添加项目根目录作为包含路径
    if not compile_to_objs(cpp_files, obj_dir, include_dirs, max_workers=args.jobs):
        print("\033[31m编译库文件失败，终止构建\033[0m")
        sys.exit(1)

    # 步骤 3: 构建示例
    print("\n\033[34m构建示例程序...\033[0m")
    examples_dir = os.path.join(project_root, '.examples')
    success_count = build_examples(examples_dir, obj_dir, max_workers=args.jobs)

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
