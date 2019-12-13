# static-dynamic-library-learning
 2019 summer vacation
>This is temporarily using code segment to show txt content.
```
gcc:
>预处理文件*.i
gcc -E -P -i <input file> -o <output prerprocessed file>.i
                           ^否则输出到屏幕上
>汇编*.s
gcc -S -masm=[att|intel] <input file> [-o <output file>.s] 
                                        ^不会输出到屏幕上

Disassembly:
>
objdump -D [-M intel] <input binary file(ELF)>
objdump.exe -D [-M **intel**] <input binary file(PE/COFF)>
>对特定节反汇编
objdump -x -j <section> <ELF>

readelf

符号检测工具：
>linux查看动态库符号
nm libvisibility.so
>查看外部可见符号
nm -g <lib>

构建动态库
gcc -fPIC <inputfile> -shared -o <output lib name>.so
    ^必需             ^一定要

-Linux
	构建过程中控制符号可见性
	>所有代码
	cflags += -fvisibility=<hidden|default> [-fvisibility-inlines-hidden]
											 ^对C++才有用，隐藏所有内联函数符号
	>对单个符号
	__attribute__((visibility("<hidden|default>")))
				  ^要两个括号
	>对单个符号或多个符号
	#pragma GCC visibility push([hidden|default])
	...
	#pragma GCC visibility pop
			^这东西不能少
	>与Windows一样严格
	cflags += -Wl,--no-undefined
	           ^传递给链接器的选项

-Windows
	>添加__declspec(dllexport), __declspec(dllimport)关键字到函数修饰中
	>.def模块定义文件

动态库的加载
>加载时动态链接-->从程序启动开始
>运行时动态链接
-Linux
#include <dlfcn.h>
1.void* dlopen("path/to/dll", mode);
                             ^打开方式
			<RTLD_LAZY/_NOW|_LOCAL/_GLOBAL|_NODELETE/_NOLOAD/_DEEPBIND>
2.(FuncRetType)dlsym(handle, "symbolname");
3.fprintf(stderr,"%s\n",dlerror());
4.void dlclose(handle);
clfags += -ldl

-Windows
#include<windows.h>
1.(HINSTANCE)LoadLibraryA("path/to/dll");
2.(FuncRetType)GetProcAddress(handle, "symbolname");
3.FreeLibrary(handle);
4.GetLastError(); // ----usage?

运行时动态库文件定位
>预加载库
/etc/ld.so.preload
export LD_PRELOAD=path/to/dynlib.so[:$LD_PRELOAD]
                ^ ^不能有空格    ^具体到文件（所以叫预加载**库**）
>rpath & runpath
export LD_RUN_PATH=path/to/dynlib[:$LD_RUN_PATH]
                 ^ ^不能有空格
export LD_LIBRARY_PATH=path/to/dynlib[:$LD_LIBRARY_PATH]
                     ^ ^不能有空格
gcc [-Wl,]-Rpath/to/runtime/dll/[:other/possible/path/to/dll] [-Wl,--enable-new-dtags]
     ^传递给链接器的选项              ^设置rpath与runpath值相同
	 
-修改runpath值（不能超过原有长度，否则会造成不可恢复的错误）
patchelf --set-rpath <new rpath> <exec file>

-优先级（预加载库最高）
rpath(if no runpath exist) > LD_LIBRARY_PATH > runpath(if exist) > ld.so.cache > /lib:/usr/lib(默认库路径)
                                                                        ^               ^
																		+-------+-------|
																		        |如果编译时使用了-z nodeflib的链接选项不查找这两项

链接器重定位提示
>readelf -r <ELF file>
>objdump -R <ELF file>

重复符号优先级
>重复符号位置：目标文件和静态库（不允许出现重复符号）>导出的动态符号>不参与链接的局部符号或不可见的符号
>链接时指定的动态库链接顺序

动态库版本管理
1.基于soname
>动态库嵌入soname
gcc -fPIC -shared <input file> -Wl,soname,<lib soname> -o <lib full name>
>客户二进制文件嵌入soname
ldconfig -n . 或手动设置软链接 ln -s libdemo.so.1.0.0 libdemo.so.1 
ln -s libdemo.so.1 libdemo.so（为了下面方便用-L-l规则链接动态库）
gcc <input file> [-Wl,-R<path>] [-Wl,]-L<path> [-Wl,]-l<truncated lib name>
>
2.基于符号(需两种方法结合使用)
>versionScript
clfags += -Wl,--version-script,<version script name>
语法：
[<版本节点>]{
	<some descriptors here>
};
>global:对外提供的符号；local:内部使用的符号；函数名称之后要跟一个';'
>通配符：*;first*;
>连接说明符：extern"C"{}或extern"C++"{}
>namespace: 
...
extern"C++"{
	libabc_namespace::*;
}
...
>匿名节点：控制符号可见性

查看所有版本信息节的信息
readelf -V <ELF file>
.gnu.version 所有相关的符号版本控制信息的汇总列表
.gnu.version_d(efinition)此动态库的符号版本控制信息
.gnu.version_r(eference)引用其他库的符号版本控制信息

>.symver
e.g.
__asm__(".symver first_1_0,first@LIBDEMO_1.0");
type first_1_0(type var, ...)
{
	...
}
__asm__(".symver first_2_0,first@@LIBDEMO_2.0");
                                 ^两个‘@’表示‘default’，相同符号只能有一个‘default’
type first_2_0(type var, ...)
{                       ^函数列表和返回值（签名）可以不同，须在头文件中修改声明，可用宏定义
	...
}

构建可执行的动态库
1.插入如下代码
#include <unistd.h> // required by _exit(0)
// 也可以用stdlib.h中的exit(0)
// determine where the dyn linker is
#ifdef __LP64__
const char <name like service_interp>[] __attribute__((section(".interp"))) = "/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2";
#else
const char <name like service_interp>[] __attribute__((section(".interp"))) = "/lib/ld-linux.so.2";
#endif

...

type <entry func name>(var list,...)
{
	<do sth. to show you've heen here>
	_exit(0);// noreturn type, or use exit(0)
}
2.构建代码(Ubuntu虚拟机上可运行，但WSL上运行不了，未知原因)
gcc -fPIC -shared <input file> -Wl,-e,<entry func name> -o <output lib name>

弱符号
符号名前加上 __attribute__((weak))
```
