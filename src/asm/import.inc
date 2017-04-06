;
; Импортируемые функции
; вынесено в отдельный файл, чтобы не захламлять код основной программы
;

  library kernel, 'KERNEL32.DLL',\
	  user	, 'USER32.DLL'

  import kernel,\
	 lstrcmpA	    , 'lstrcmpA',\
	 lstrcpyA	    , 'lstrcpyA',\
	 lstrlenA	    , 'lstrlenA',\
	 lstrcatA	    , 'lstrcatA',\
	 _lwrite	    , '_lwrite',\
	 _lopen 	    , '_lopen',\
	 _llseek	    , '_llseek',\
	 _lread 	    , '_lread',\
	 _lclose	    , '_lclose',\
	 lstrcpynA	    , 'lstrcpynA',\
	 LocalAlloc	    , 'LocalAlloc',\
	 LocalFree	    , 'LocalFree',\
	 LocalUnlock	    , 'LocalUnlock',\
	 GlobalFree	    , 'GlobalFree',\
	 GetSystemDirectoryA, 'GetSystemDirectoryA'

  import user,\
	 CharUpperA	    , 'CharUpperA',\
	 CharToOemA	    , 'CharToOemA',\
	 wsprintfA	    , 'wsprintfA',\
	 IsCharAlphaA	    , 'IsCharAlphaA',\
	 IsCharAlphaNumericA, 'IsCharAlphaNumericA'