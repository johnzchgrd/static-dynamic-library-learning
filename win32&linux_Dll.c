#include <stdio.h>
#ifdef __linux__
#include <dlfcn.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif

typedef int (*FT)(int);

int main()
{
#ifdef __linux__
	void*pHandle = dlopen("./libdemo.so",RTLD_LAZY);
	if(pHandle==NULL)
		{
			fprintf(stderr,"%s\n",dlerror());
			return -1;
		}
	FT fun1 = (FT)dlsym(pHandle,"first");


	if(fun1==NULL)
		{
			fprintf(stderr,"%s\n",dlerror());
			goto end;
		}
	FT fun2 = (FT)dlsym(pHandle,"second");
	if(fun2==NULL)
		{
			fprintf(stderr,"%s\n",dlerror());
			goto end;
		}
	FT fun3 = (FT)dlsym(pHandle,"third");
	if(fun3==NULL)
		{
			fprintf(stderr,"%s\n",dlerror());
			goto end;
		}

	printf("First: %d\n", fun1(1));
	printf("Second: %d\n", fun2(1));
	printf("Third: %d\n", fun3(1));

end:
	dlclose(pHandle);
#endif
#ifdef WIN32
	HINSTANCE dlHandle = LoadLibraryA("C:\\Users\\Johnzchgrd\\source\\repos\\Dll1\\Debug\\Dll1.dll");
	if(dlHandle==NULL)
	{
		printf("Dll Load Error.\n");
		return -1;
	}
	FT wfun1 = (FT)GetProcAddress(dlHandle,"first");
	if(wfun1==NULL)
	{
		printf("Cannot find symbol 1!\n");
		FreeLibrary(dlHandle);
		return -1;
	}
	FT wfun2 = (FT)GetProcAddress(dlHandle,"second");
	if(wfun2==NULL)
	{
		printf("Cannot find symbol 2!\n");
		FreeLibrary(dlHandle);
		return -1;
	}
	printf("First: %d\n", wfun1(2));
	printf("Second: %d\n", wfun2(2));
#endif
	return 0;
}