// MemFiller.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	// コンソール出力か?
	bool useConsole = _isatty(_fileno(stdout)) != 0;

	// メモリ状況の取得
	MEMORYSTATUSEX meminfo = {0};
	meminfo.dwLength = sizeof(MEMORYSTATUSEX);

	if (!GlobalMemoryStatusEx(&meminfo)) {
		_ftprintf(
			stderr,
			_T("GlobalMemoryStatusEx: errorcode: %ld\n"),
			GetLastError()
			);
		exit(1);
	}

	// 物理メモリ使用状況の表示
	size_t unit = 1024 * 1024; // 1 MegaBytes

	_ftprintf(
		stdout,
		_T("Phys: %I64d/%I64d MBytes\n"),
		meminfo.ullAvailPhys / unit,
		meminfo.ullTotalPhys / unit
		);

	// 最大ループ回数 = 空き物理メモリ(MB単位)
	size_t maxLoopCount = static_cast<size_t>(meminfo.ullAvailPhys / unit);
	if (argc > 1) {
		maxLoopCount = _ttoi(argv[1]);
	}

	// 現在のワーキングセットの取得
	HANDLE hProcess = GetCurrentProcess();
	SIZE_T curMinSize = 0;
	SIZE_T curMaxSize = 0;
	GetProcessWorkingSetSize(hProcess, &curMinSize, &curMaxSize);

	// ワーキングセットの設定
	SIZE_T maxMem = (maxLoopCount + 1) * unit + curMaxSize;
	if (!SetProcessWorkingSetSize(
		hProcess, maxMem, maxMem)) {
		_ftprintf(
			stderr,
			_T("SetProcessWorkingSetSizeEx: errorcode: %ld\n"),
			GetLastError()
			);
		// とりあえず継続してみる
	}

	// メモリを最大まで確保する.
	if (maxLoopCount > 0) {
		std::vector<void *> buf(maxLoopCount);
		for (size_t loopCount = 0; loopCount < maxLoopCount; loopCount++) {

			// コンソール出力であればプログレス表示
			if (useConsole) {
				_ftprintf(stderr, _T("allocating... %ld/%ld MBytes \r"), loopCount + 1, maxLoopCount);
			}

			// メモリの確保
			void *p = VirtualAlloc(NULL, unit, MEM_COMMIT, PAGE_READWRITE);
			if (p == NULL) {
				DWORD errcode = GetLastError();
				if (useConsole) {
					_ftprintf(stderr, _T("\n"));
				}
				_ftprintf(stderr, _T("VirtualAlloc: errorcode: %ld\n"), errcode);
				break;
			}
			
			// 確保済みメモリとして記憶する.
			buf[loopCount] = p;

			// 物理メモリをロックする
			if (!VirtualLock(p, unit)) {
				DWORD errcode = GetLastError();
				if (useConsole) {
					_ftprintf(stderr, _T("\n"));
				}
				_ftprintf(stderr, _T("VirtualLock: errorcode: %ld\n"), errcode);
				break;
			}
		}
		if (useConsole) {
			_ftprintf(stderr, _T("\n"));
		}

		// メモリを解放する
		for (size_t loopCount = 0; loopCount < maxLoopCount; loopCount++) {
			void *p = buf[loopCount];
			if (p) {
				VirtualUnlock(p, unit);
				VirtualFree(p, unit, MEM_DECOMMIT);
			}
		}
	}

	return 0;
}

