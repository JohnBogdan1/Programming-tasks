/**
 * Operating Systems 2013-2017 - Assignment 2
 *
 * Name, Group
 * Ion Bogdan-Ionut, 332CB
 */

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "utils.h"

#define READ		0
#define WRITE		1

typedef struct {
	command_t *c;
	command_t *father;
	HANDLE h[2];
} MYPARAMS, *PMYPARAMS;

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	/* execute cd */
	DWORD dwRet;

	char *dir_path = get_word(dir);

	dwRet = SetCurrentDirectory(dir_path);

	if (dwRet == 0) {
		fprintf(stderr, "Missing directory: %s\n", dir_path);
		return -1;
	}

	return 0;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{
	/* execute exit/quit */

	return SHELL_EXIT; /* actual exit code */
}

/*
 * @psi		- STATRTUPINFO of the child process
 * @hFile	- file handle for redirect
 * @opt		- redirect option is one of the following
 *		 STD_INPUT_HANDLE,STD_OUTPUT_HANDLE, STD_ERROR_HANDLE
 */
static VOID RedirectHandle(STARTUPINFO *psi, HANDLE hFile, INT opt)
{
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	/* Redirect */

	psi->dwFlags |= STARTF_USESTDHANDLES;

	switch (opt) {
	case STD_INPUT_HANDLE:
		psi->hStdInput = hFile;
		break;
	case STD_OUTPUT_HANDLE:
		psi->hStdOutput = hFile;
		break;
	case STD_ERROR_HANDLE:
		psi->hStdError = hFile;
		break;
	}
}

static HANDLE MyCreateFile(LPTSTR filename, DWORD dwDesiredAccess,
	DWORD dwCreationDisposition, DWORD dwShareMode)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;

	hFile = CreateFile(
		filename,
		dwDesiredAccess,
		dwShareMode,
		&sa,
		dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DIE(hFile == INVALID_HANDLE_VALUE, "CreateFile");

	return hFile;
}

static HANDLE *redirections(simple_command_t *s, HANDLE hFile,
	STARTUPINFO *si, int *size)
{

	LPTSTR filename;
	HANDLE *handles;

	handles = (HANDLE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
		(*size) * sizeof(HANDLE));

	while (true) {
		if (s->in != NULL) {
			filename = get_word(s->in);

			hFile = MyCreateFile(filename, GENERIC_READ,
				OPEN_EXISTING, FILE_SHARE_READ);

			handles[*size - 1] = hFile;

			handles = HeapReAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY,
				handles, (++(*size)) * sizeof(HANDLE));

			RedirectHandle(si, hFile, STD_INPUT_HANDLE);

			s->in = s->in->next_word;

		} else if (s->out != NULL && s->err != NULL &&
			!strcmp(s->out->string, s->err->string)) {

			filename = get_word(s->out);

			hFile = MyCreateFile(filename, GENERIC_WRITE,
				OPEN_ALWAYS, FILE_SHARE_WRITE);

			SetFilePointer(
				hFile,
				0,
				NULL,
				FILE_END
			);

			handles[*size - 1] = hFile;

			handles = HeapReAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY, handles,
				(++(*size)) * sizeof(HANDLE));

			RedirectHandle(si, hFile, STD_OUTPUT_HANDLE);

			hFile = MyCreateFile(filename, GENERIC_WRITE,
				CREATE_ALWAYS, FILE_SHARE_WRITE);


			handles[*size - 1] = hFile;

			handles = HeapReAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY, handles,
				(++(*size)) * sizeof(HANDLE));

			RedirectHandle(si, hFile, STD_ERROR_HANDLE);

			s->out = s->out->next_word;
			s->err = s->err->next_word;

		} else if (s->out != NULL) {
			filename = get_word(s->out);

			if (s->io_flags == IO_OUT_APPEND) {
				hFile = MyCreateFile(filename,
					GENERIC_WRITE, OPEN_ALWAYS,
					FILE_SHARE_WRITE);

				SetFilePointer(
					hFile,
					0,
					NULL,
					FILE_END
				);

			} else {
				hFile = MyCreateFile(filename, GENERIC_WRITE,
					CREATE_ALWAYS, FILE_SHARE_READ);
			}

			handles[*size - 1] = hFile;

			handles = handles = HeapReAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY, handles,
				(++(*size)) * sizeof(HANDLE));

			RedirectHandle(si, hFile, STD_OUTPUT_HANDLE);

			s->out = s->out->next_word;

		} else if (s->err != NULL) {
			filename = get_word(s->err);

			if (s->io_flags == IO_ERR_APPEND) {
				hFile = MyCreateFile(filename, GENERIC_WRITE,
					OPEN_ALWAYS, FILE_SHARE_WRITE);

				SetFilePointer(
					hFile,
					0,
					NULL,
					FILE_END
				);

			} else {
				hFile = MyCreateFile(filename, GENERIC_WRITE,
					CREATE_ALWAYS, FILE_SHARE_WRITE);
			}

			handles[*size - 1] = hFile;

			handles = HeapReAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY, handles,
				(++(*size)) * sizeof(HANDLE));

			RedirectHandle(si, hFile, STD_ERROR_HANDLE);

			s->err = s->err->next_word;

		}

		if (s->in == NULL && s->out == NULL && s->err == NULL)
			return handles;
	}
}

/**
 * Parse and execute a simple command, by either creating a new processing or
 * internally process it.
 */
bool parse_simple(simple_command_t *s, int level, command_t *father, HANDLE *h)
{
	char *int_cmd = NULL;
	DWORD dwRet;
	BOOL bRes;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	LPTSTR command = NULL;
	int size = 1, i;
	HANDLE *handles = NULL, my_handle = INVALID_HANDLE_VALUE;
	HANDLE read = INVALID_HANDLE_VALUE, write = INVALID_HANDLE_VALUE;
	HANDLE old_stdout, old_stdin;

	/* sanity checks */
	if (s == NULL)
		return -1;
	else if (s->verb == NULL)
		return -1;

	old_stdin = GetStdHandle(STD_INPUT_HANDLE);
	old_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	/* standard handles */
	(&si)->hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	(&si)->hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	(&si)->hStdError = GetStdHandle(STD_ERROR_HANDLE);

	/* if builtin command, execute the command */
	int_cmd = get_word(s->verb);

	if (!strncmp("exit", int_cmd, strlen("exit")) || !strncmp("quit",
		int_cmd, strlen("quit"))) {

		dwRet = shell_exit();

		return dwRet;
	}

	if (!strncmp("cd", int_cmd, strlen("cd"))) {

		/* it means the "cd" command has a param: directory name */
		if (s->params != NULL) {
			handles = redirections(s, my_handle, &si, &size);

			dwRet = shell_cd(s->params);

			for (i = 0; i < size - 1; i++)
				CloseHandle(handles[i]);
		}
		return dwRet;
	}

	/* if variable assignment, execute the assignment and return
	 * the exit status
	 */
	if (s->verb->next_part != NULL &&
		!strncmp("=", s->verb->next_part->string, strlen("="))) {

		if (strcmp(s->verb->string, "")) {
			dwRet = SetEnvironmentVariable(s->verb->string,
				s->verb->next_part->next_part->string);
			DIE(dwRet == 0, "Invalid name(name=null).");

			if (dwRet)
				return 0;
			return 1;
		}
	}

	/* if external command:
	 *  1. set handles
	 *  2. redirect standard input / output / error
	 *  3. run command
	 *  4. get exit code
	 */

	if (h != NULL) {

		if (h[1] != INVALID_HANDLE_VALUE) {
			write = GetStdHandle(STD_OUTPUT_HANDLE);
			RedirectHandle(&si, h[1], STD_OUTPUT_HANDLE);
		}
		
		if (h[0] != INVALID_HANDLE_VALUE) {
			read = GetStdHandle(STD_INPUT_HANDLE);
			RedirectHandle(&si, h[0], STD_INPUT_HANDLE);
		}
	}

	handles = redirections(s, my_handle, &si, &size);

	command = get_argv(s);

	bRes = CreateProcess(
		NULL,
		command,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&si,
		&pi);

	if (!bRes) {
		fprintf(stderr, "Execution failed for '%s'\n", command);
		fflush(stderr);
		bRes = GetExitCodeProcess(pi.hProcess, &dwRet);
	} else {

		/* close pipe's ends */
		if (h != NULL) {
			if (h[1] != INVALID_HANDLE_VALUE) {
				CloseHandle(h[1]);
				h[1] = INVALID_HANDLE_VALUE;
				SetStdHandle(STD_OUTPUT_HANDLE, write);
				SetStdHandle(STD_INPUT_HANDLE, old_stdin);

			}
			if (h[0] != INVALID_HANDLE_VALUE) {

				/* for multiple pipes, close the write end */
				/* close it in cmd2's thread */
				if (h[1] != INVALID_HANDLE_VALUE) {
					CloseHandle(h[1]);
					h[1] = INVALID_HANDLE_VALUE;
					SetStdHandle(STD_OUTPUT_HANDLE,
						old_stdout);
				}

				CloseHandle(h[0]);
				h[0] = INVALID_HANDLE_VALUE;
				SetStdHandle(STD_INPUT_HANDLE, read);
				SetStdHandle(STD_OUTPUT_HANDLE, old_stdout);
			}
		}

		/* wait for the process */
		dwRet = WaitForSingleObject(pi.hProcess, INFINITE);
		DIE(dwRet == WAIT_FAILED, "WaitForSingleObject");

		bRes = GetExitCodeProcess(pi.hProcess, &dwRet);
	    DIE(bRes == FALSE, "GetExitCode");

	    CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	if (handles != NULL) {
		for (i = 0; i < size - 1; i++) {
			CloseHandle(handles[i]);
			handles[i] = INVALID_HANDLE_VALUE;
		}
	}

	if (handles != NULL) {
		HeapFree(GetProcessHeap(), 0, handles);
		handles = NULL;
	}

	return dwRet; /* actual exit status */
}

/**
 * Thread's function.
 */
static DWORD WINAPI thread_func(command_t *c)
{

	int ret;

	ret = parse_command(c, 0, NULL, NULL);

	return ret;

}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_in_parallel(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* execute cmd1 and cmd2 simultaneously */
	/* I create two threads that execute the commands 1 and 2 */

	HANDLE thread_arr[2];
	DWORD thread_id_arr[2], dwRet;
	BOOL bRes;

	thread_arr[0] = CreateThread(
		NULL,
		0,
		thread_func,
		cmd1,
		0,
		&thread_id_arr[0]);

	DIE(!thread_arr[0], "CreateThread");

	thread_arr[1] = CreateThread(
		NULL,
		0,
		thread_func,
		cmd2,
		0,
		&thread_id_arr[1]);

	DIE(!thread_arr[1], "CreateThread");

	/* wait for threads to finish */

	dwRet = WaitForMultipleObjects(2, thread_arr, TRUE, INFINITE);
	DIE(dwRet == WAIT_FAILED, "WaitForThreads");

	bRes = GetExitCodeThread(thread_arr[0], &dwRet);
	DIE(bRes == FALSE, "GetExitCode");

	bRes = GetExitCodeThread(thread_arr[1], &dwRet);
	DIE(bRes == FALSE, "GetExitCode");

	CloseHandle(thread_arr[0]);
	CloseHandle(thread_arr[1]);

	return dwRet; /* actual exit status */
}

/**
 * Thread's function.
 */
static DWORD WINAPI pipe_thread_func(PMYPARAMS param_array)
{

	int ret;

	ret = parse_command(param_array->c, 0, param_array->father,
		param_array->h);

	return ret;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* redirect the output of cmd1 to the input of cmd2 */

	SECURITY_ATTRIBUTES sa;

	HANDLE hRead = INVALID_HANDLE_VALUE, hWrite = INVALID_HANDLE_VALUE;
	HANDLE thread_arr[2];
	DWORD thread_id_arr[2], dwRet;
	BOOL bRes;
	HANDLE old_stdout, old_stdin;

	PMYPARAMS p_thread_params[2];

	p_thread_params[0] = (PMYPARAMS) HeapAlloc(GetProcessHeap(),
		HEAP_ZERO_MEMORY, sizeof(MYPARAMS));

	DIE(p_thread_params[0] == NULL, "Params allocation.");

	p_thread_params[1] = (PMYPARAMS) HeapAlloc(GetProcessHeap(),
		HEAP_ZERO_MEMORY, sizeof(MYPARAMS));

	DIE(p_thread_params[1] == NULL, "Params allocation.");

	/* initialize arrays */

	p_thread_params[0]->h[0] = INVALID_HANDLE_VALUE;
	p_thread_params[0]->h[1] = INVALID_HANDLE_VALUE;
	p_thread_params[1]->h[0] = INVALID_HANDLE_VALUE;
	p_thread_params[1]->h[1] = INVALID_HANDLE_VALUE;

	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;

	old_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	old_stdin = GetStdHandle(STD_INPUT_HANDLE);

	bRes = CreatePipe(&hRead, &hWrite, &sa, 0);
	DIE(bRes == FALSE, "CreatePipe");

	/************command 1*************/

	p_thread_params[0]->c = cmd1;
	p_thread_params[0]->father = father;
	p_thread_params[0]->h[1] = hWrite;

	/* if cmd1 is a pipe, redirect stdout to hWrite */
	if (cmd1->op == OP_PIPE) {
		SetStdHandle(STD_OUTPUT_HANDLE, hWrite);
		p_thread_params[1]->h[1] = hWrite;
	}

	thread_arr[0] = CreateThread(
		NULL,
		0,
		pipe_thread_func,
		p_thread_params[0],
		0,
		&thread_id_arr[0]);

	DIE(!thread_arr[0], "CreateThread");

	/************command 2*************/

	p_thread_params[1]->c = cmd2;
	p_thread_params[1]->father = father;
	p_thread_params[1]->h[0] = hRead;

	thread_arr[1] = CreateThread(
		NULL,
		0,
		pipe_thread_func,
		p_thread_params[1],
		0,
		&thread_id_arr[1]);

	DIE(!thread_arr[1], "CreateThread");

	/************wait*************/

	/************close & free*************/

	dwRet = WaitForMultipleObjects(2, thread_arr, TRUE, INFINITE);
	DIE(dwRet == WAIT_FAILED, "WaitForThreads");

	if (cmd1->op == OP_PIPE)
		SetStdHandle(STD_OUTPUT_HANDLE, old_stdout);

	bRes = GetExitCodeThread(thread_arr[0], &dwRet);
	DIE(bRes == FALSE, "GetExitCode");

	bRes = GetExitCodeThread(thread_arr[1], &dwRet);
	DIE(bRes == FALSE, "GetExitCode");

	CloseHandle(thread_arr[0]);

	if (p_thread_params[0] != NULL) {
		HeapFree(GetProcessHeap(), 0, p_thread_params[0]);
		p_thread_params[0] = NULL;
	}

	CloseHandle(thread_arr[1]);

	if (p_thread_params[1] != NULL) {
		HeapFree(GetProcessHeap(), 0, p_thread_params[1]);
		p_thread_params[1] = NULL;
	}

	SetStdHandle(STD_OUTPUT_HANDLE, old_stdout);
	SetStdHandle(STD_INPUT_HANDLE, old_stdin);

	return dwRet; /* actual exit status */
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father, void *h)
{
	int ret;

	/* sanity checks */
	if (c == NULL)
		return -1;
	else if (c->scmd == NULL && c->cmd1 == NULL && c->cmd2 == NULL)
		return -1;

	if (c->op == OP_NONE) {
		/* execute a simple command */
		if (c->scmd != NULL && c->cmd1 == NULL && c->cmd2 == NULL)
			ret = parse_simple(c->scmd, level, father, h);

		return ret; /* actual exit code of command */
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* execute the commands one after the other */

		if (c->scmd == NULL && c->cmd1 != NULL && c->cmd2 != NULL) {
			parse_command(c->cmd1, level, c, h);

			parse_command(c->cmd2, level, c, h);
		}

		break;

	case OP_PARALLEL:
		/* execute the commands simultaneously */
		if (c->scmd == NULL && c->cmd1 != NULL && c->cmd2 != NULL)
			ret = do_in_parallel(c->cmd1, c->cmd2, level, c);

		break;

	case OP_CONDITIONAL_NZERO:
		/* execute the second command only if the first one
		 * returns non zero
		 */
		if (c->scmd == NULL && c->cmd1 != NULL && c->cmd2 != NULL) {
			ret = parse_command(c->cmd1, level, c, h);

			if (ret != 0)
				ret = parse_command(c->cmd2, level, c, h);
		}

		break;

	case OP_CONDITIONAL_ZERO:
		/* execute the second command only if the first one
		 * returns zero
		 */

		if (c->scmd == NULL && c->cmd1 != NULL && c->cmd2 != NULL) {
			ret = parse_command(c->cmd1, level, c, h);

			if (ret == 0)
				ret = parse_command(c->cmd2, level, c, h);
		}

		break;

	case OP_PIPE:
		/* redirect the output of the first command to the
		 * input of the second
		 */
		/* if (c->scmd == NULL && c->cmd1 != NULL && c->cmd2 != NULL)
		 *	ret = do_on_pipe(c->cmd1, c->cmd2, level, c);
		 */
		break;

	default:
		return SHELL_EXIT;
	}

	return ret; /* actual exit code of command */
}
