// MemFiller.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	// �������󋵂̎擾
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

	// �����������g�p�󋵂̕\��
	size_t unit = 1024 * 1024; // 1 MegaBytes

	_ftprintf(
		stdout,
		_T("Phys: %I64d/%I64d MBytes\n"),
		meminfo.ullAvailPhys / unit,
		meminfo.ullTotalPhys / unit
		);

	// �ő僋�[�v�� = �󂫕���������(MB�P��)
	size_t maxLoopCount = static_cast<size_t>(meminfo.ullAvailPhys / unit);
	if (argc > 1) {
		maxLoopCount = _ttoi(argv[1]);
	}

	// ���[�L���O�Z�b�g�̐ݒ�
	size_t maxMem = (maxLoopCount + 1) * unit; // 1MB�]���Ɋm��
	if (!SetProcessWorkingSetSize(
		GetCurrentProcess(), maxMem, maxMem)) {
		_ftprintf(
			stderr,
			_T("SetProcessWorkingSetSizeEx: errorcode: %ld\n"),
			GetLastError()
			);
		// �Ƃ肠�����p�����Ă݂�
	}

	// �R���\�[���o�͂�?
	bool useConsole = _isatty(_fileno(stdout)) != 0;

	// ���������ő�܂Ŋm�ۂ���.
	if (maxLoopCount > 0) {
		std::vector<void *> buf(maxLoopCount);
		for (size_t loopCount = 0; loopCount < maxLoopCount; loopCount++) {

			// �R���\�[���o�͂ł���΃v���O���X�\��
			if (useConsole) {
				_ftprintf(stderr, _T("allocating... %ld/%ld MBytes \r"), loopCount + 1, maxLoopCount);
			}

			// �������̊m��
			void *p = VirtualAlloc(NULL, unit, MEM_COMMIT, PAGE_READWRITE);
			if (p == NULL) {
				DWORD errcode = GetLastError();
				if (useConsole) {
					_ftprintf(stderr, _T("\n"));
				}
				_ftprintf(stderr, _T("VirtualAlloc: errorcode: %ld\n"), errcode);
				break;
			}
			
			// �m�ۍς݃������Ƃ��ċL������.
			buf[loopCount] = p;

			// ���������������b�N����
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

		// ���������������
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

