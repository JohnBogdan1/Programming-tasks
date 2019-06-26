/**
 * Operating Systems 2013-2017 - Assignment 2
 *
 * Name, Group
 * Ion Bogdan-Ionut, 332CB
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <unistd.h>

#include "cmd.h"
#include "utils.h"

#define READ		0
#define WRITE		1

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	/* execute cd */
	int ret;
	char *dir_path = NULL;

	/* used get_word() because there could be more parts in the string */
	dir_path = get_word(dir);

	/* change current directory using chdir */
	ret = chdir(dir_path);

	/* directory missing */
	if (ret  == -1)
		fprintf(stderr, "Missing directory: %s\n", dir->string);

	free(dir_path);

	return ret;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{
	/* execute exit/quit */

	return SHELL_EXIT; /* return actual exit code */
}

/**
* Do the file redirections.
*/
static int *redirections(simple_command_t *s)
{
	int fd, ret, i;
	char *filename = NULL;
	static int io_fds[3]; /* save 0, 1, 2 fd in this array */

	/* initialize */
	for (i = 0; i < 3; i++)
		io_fds[i] = -1;

	/* while there are redirections in command */
	/* used get_word() because there could be more parts in the string */
	while (true) {
		if (s->in != NULL) { /* stdin redirection("<") */
			/* open the file, only read*/
			filename = get_word(s->in);
			fd = open(filename, O_RDONLY);

			DIE(fd < 0, "Open file.");

			/* save stdin */
			if (io_fds[0] == -1)
				io_fds[0] = dup(STDIN_FILENO);

			/* redirect stdin to file  and close it*/
			ret = dup2(fd, STDIN_FILENO);
			DIE(ret < 0, "dup2");

			close(fd);

			free(filename);

			/* go next */
			s->in = s->in->next_word;

		} else if (s->out != NULL && s->err != NULL &&
			!strcmp(s->out->string, s->err->string)) {
			/* stdout and stderr redirection("&>") */

			/* s->out and s->err return the same filename */
			/* open the file in append mode and redirect stdout */
			filename = get_word(s->out);
			fd = open(filename, O_WRONLY | O_CREAT | O_APPEND,
				0644);
			DIE(fd < 0, "Open file.");

			/* save stdout */
			if (io_fds[1] == -1)
				io_fds[1] = dup(STDOUT_FILENO);

			/* redirect stdout to file and close it*/
			ret = dup2(fd, STDOUT_FILENO);
			DIE(ret < 0, "dup2");

			close(fd);

			/* open the file in trunc mode and redirect stderr */
			fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
				0644);
			DIE(fd < 0, "Open file.");

			/* save stderr */
			if (io_fds[2] == -1)
				io_fds[2] = dup(STDERR_FILENO);

			/* redirect stderr to file and close it */
			ret = dup2(fd, STDERR_FILENO);
			DIE(ret < 0, "dup2");

			close(fd);

			free(filename);

			/* go next */
			s->out = s->out->next_word;
			s->err = s->err->next_word;

		} else if (s->out != NULL) { /* stdout redirection(">")*/

			/* open the file in trunc / append mode */
			filename = get_word(s->out);
			if (s->io_flags == IO_REGULAR) {
				fd = open(filename, O_WRONLY | O_CREAT |
					O_TRUNC, 0644);
				DIE(fd < 0, "Open file.");
			} else if (s->io_flags == IO_OUT_APPEND) {
				fd = open(filename, O_WRONLY | O_CREAT |
					O_APPEND, 0644);
				DIE(fd < 0, "Open file.");
			}

			/* save stdout */
			if (io_fds[1] == -1)
				io_fds[1] = dup(STDOUT_FILENO);

			/* redirect stdout to file and close it*/
			ret = dup2(fd, STDOUT_FILENO);
			DIE(ret < 0, "dup2");

			close(fd);

			free(filename);

			/* go next */
			s->out = s->out->next_word;

		} else if (s->err != NULL) {
			/* open the file in trunc / append mode */
			filename = get_word(s->err);
			if (s->io_flags == IO_REGULAR) {
				fd = open(filename, O_WRONLY | O_CREAT |
					O_TRUNC, 0644);
				DIE(fd < 0, "Open file.");
			} else if (s->io_flags == IO_ERR_APPEND) {
				fd = open(filename, O_WRONLY | O_CREAT |
					O_APPEND, 0644);
				DIE(fd < 0, "Open file.");
			}

			/* save stderr */
			if (io_fds[2] == -1)
				io_fds[2] = dup(STDERR_FILENO);

			/* redirect stderr to file and close it*/
			ret = dup2(fd, STDERR_FILENO);
			DIE(ret < 0, "dup2");

			close(fd);

			free(filename);

			/* go next */
			s->err = s->err->next_word;
		}

		/* if all are NULL, then return */
		if (s->in == NULL && s->out == NULL && s->err == NULL)
			return io_fds;
	}
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	char *int_cmd = NULL;
	char **argv = NULL;
	int ret, i, rc;
	pid_t pid, wait_ret;
	int status, size;
	int *fds;

	/* sanity checks */
	if (s == NULL)
		return -1;
	else if (s->verb == NULL)
		return -1;

	/* if builtin command, execute the command */
	/* get the whole command using get_word() */
	int_cmd = get_word(s->verb);

	/* if quit/exit call shell_exit */
	if (!strncmp("exit", int_cmd, strlen("exit")) || !strncmp("quit",
		int_cmd, strlen("quit"))) {

		free(int_cmd);
		int_cmd = NULL;

		ret = shell_exit();

		return ret;
	}

	/* if cd(change directory), call shell_cd
	 * , but before it, do the redirections if there are any and after cd
	 * restore stdin, stdout, stderr fd
	 */
	if (!strncmp("cd", int_cmd, strlen("cd"))) {
		free(int_cmd);
		int_cmd = NULL;

		/* it means the "cd" command has a param: directory name */
		if (s->params != NULL) {
			fds = redirections(s);

			ret = shell_cd(s->params);

			/* restore file descriptors */
			if (fds[0] != -1) {
				rc = dup2(fds[0], STDIN_FILENO);
				DIE(rc < 0, "dup2");
				close(fds[0]);
			}

			if (fds[1] != -1) {
				rc = dup2(fds[1], STDOUT_FILENO);
				DIE(rc < 0, "dup2");
				close(fds[1]);
			}

			if (fds[2] != -1) {
				rc = dup2(fds[2], STDERR_FILENO);
				DIE(rc < 0, "dup2");
				close(fds[2]);
			}
		}
		return ret;
	}

	/* if variable assignment, execute the assignment and return
	 * the exit status
	 */

	if (s->verb->next_part != NULL &&
		!strncmp("=", s->verb->next_part->string, strlen("="))) {

		free(int_cmd);
		int_cmd = NULL;

		if (strcmp(s->verb->string, "")) {
			ret = setenv(s->verb->string,
				s->verb->next_part->next_part->string, 1);

			DIE(ret < 0, "Invalid name(name=null).");

			return ret;
		}
	}

	/* if external command:
	 *   1. fork new process
	 *     2c. perform redirections in child
	 *     3c. load executable in child
	 *   2. wait for child
	 *   3. return exit status
	 */

	/* create process */
	pid = fork();

	switch (pid) {

	case -1:	/* error */
		DIE(pid < 0, "Fork new process.");

	case 0:		/* child process */
		/* do the redirections, if any */
		redirections(s);

		/* command arguments */
		argv = get_argv(s, &size);

		/* execute the whole command */
		execvp(int_cmd, argv);

		fprintf(stderr, "Execution failed for '%s'\n", int_cmd);
		fflush(stderr);

		/* free memory if fails */
		for (i = 0; i < size; i++)
			free(argv[i]);

		free(argv);

		if (int_cmd != NULL) {
			free(int_cmd);
			int_cmd = NULL;
		}

		exit(EXIT_FAILURE);
	default:	/* parent process */
		/* wait for the child */
		wait_ret = waitpid(pid, &status, 0);

		DIE(wait_ret < 0, "waitpid");
	}


	/* free memory */
	if (int_cmd != NULL) {
		free(int_cmd);
		int_cmd = NULL;
	}

	if (argv != NULL) {
		for (i = 0; i < size; i++)
			free(argv[i]);
		free(argv);
	}

	return status; /* return actual exit status */
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_in_parallel(command_t *cmd1, command_t *cmd2, int level,
	command_t *father)
{
	/* execute cmd1 and cmd2 simultaneously */
	/* create two process for this */
	pid_t pid1, wait_ret1, pid2, wait_ret2;
	int status, c1, c2;

	/* first process executes the first command */
	pid1 = fork();

	switch (pid1) {

	case -1:
		DIE(pid1 < 0, "Fork new process.");
		break;

	case 0:		/* child process */
		/* execute the command by calling parse_command recursively */
		c1 = parse_command(cmd1, level, father);

		/* exit depends on the command return value */
		if (c1 == 0 || c1 == SHELL_EXIT)
			exit(EXIT_SUCCESS);

		exit(EXIT_FAILURE);
		break;

	default:	/* parent process */
		break;
	}

	/* second process executes the second command */
	pid2 = fork();

	switch (pid2) {

	case -1:
		DIE(pid2 < 0, "Fork new process.");
		break;

	case 0:		/* child process */
		/* execute the command by calling parse_command recursively */
		c2 = parse_command(cmd2, level, father);

		/* exit depends on the command return value */
		if (c2 == 0 || c2 == SHELL_EXIT)
			exit(EXIT_SUCCESS);

		exit(EXIT_FAILURE);
		break;

	default:	/* parent process */
		break;
	}

	/* only parent process gets here */
	/* parent waits his children */
	wait_ret1 = waitpid(pid1, &status, 0);

	DIE(wait_ret1 < 0, "waitchild1");

	wait_ret2 = waitpid(pid2, &status, 0);

	DIE(wait_ret2 < 0, "waitchild2");

	return status; /* return actual exit status */
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level,
	command_t *father)
{
	/* redirect the output of cmd1 to the input of cmd2 */
	/* I use child and parent processes for this */
	/* parents writes to pipe and child reads from the pipe */
	pid_t pid, wait_ret;
	int status, res, ret, i;
	int filedes[2], io_fds[2]; /* with io_fds I restore stdin and stdout */

	for (i = 0; i < 2; i++)
		io_fds[i] = -1;

	/*
	 * create a pipe
	 * filedes[0] = stdin
	 * filedes[1] = stdout
	 */
	ret = pipe(filedes);

	DIE(ret < 0, "pipe");

	/* create a process */
	pid = fork();

	switch (pid) {

	case -1:
		DIE(pid < 0, "Fork new process.");
		break;

	case 0:		/* child process */
		/* close write end of pipe */
		close(filedes[1]);

		/* save stdin */
		if (io_fds[0] == -1)
			io_fds[0] = dup(STDIN_FILENO);

		/* redirect stdin to read end of pipe */
		ret = dup2(filedes[0], STDIN_FILENO);

		/* close read end of pipe */
		close(filedes[0]);

		/* execute the command */
		res = parse_command(cmd2, level, father);

		/* restore stdin */
		if (io_fds[0] != -1) {
			ret = dup2(io_fds[0], STDIN_FILENO);
			close(io_fds[0]);
		}

		/* exit depends on the command return */
		if (res == 0 || res == SHELL_EXIT)
			exit(EXIT_SUCCESS);

		exit(EXIT_FAILURE);

		break;

	default:	/* parent process */
		/* close read end of pipe */
		close(filedes[0]);

		/* save stdout */
		if (io_fds[1] == -1)
			io_fds[1] = dup(STDOUT_FILENO);

		/* redirect stdout to write end of pipe*/
		ret = dup2(filedes[1], STDOUT_FILENO);

		/* close write end of pipe */
		close(filedes[1]);

		/* execute the command */
		parse_command(cmd1, level, father);

		/* restore stdout */
		if (io_fds[1] != -1) {
			dup2(io_fds[1], STDOUT_FILENO);
			close(io_fds[1]);
		}

		/* wait for the child */
		wait_ret = waitpid(pid, &status, 0);

		DIE(wait_ret < 0, "waitchild");

		break;
	}

	return status; /* return actual exit status */
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
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
			ret = parse_simple(c->scmd, level, father);

		return ret; /* return actual exit code of command */
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* execute the commands one after the other */

		if (c->scmd == NULL && c->cmd1 != NULL && c->cmd2 != NULL) {
			parse_command(c->cmd1, level + 1, c);

			parse_command(c->cmd2, level + 1, c);
		}

		break;

	case OP_PARALLEL:
		/* execute the commands simultaneously */
		if (c->scmd == NULL && c->cmd1 != NULL && c->cmd2 != NULL)
			ret = do_in_parallel(c->cmd1, c->cmd2, level + 1, c);

		break;

	case OP_CONDITIONAL_NZERO:
		/* execute the second command only if the first one
		 * returns non zero
		 */

		if (c->scmd == NULL && c->cmd1 != NULL && c->cmd2 != NULL) {
			ret = parse_command(c->cmd1, level + 1, c);

			if (ret != 0)
				ret = parse_command(c->cmd2, level + 1, c);
		}

		break;

	case OP_CONDITIONAL_ZERO:
		/* execute the second command only if the first one
		 * returns zero
		 */
		if (c->scmd == NULL && c->cmd1 != NULL && c->cmd2 != NULL) {
			ret = parse_command(c->cmd1, level + 1, c);

			if (ret == 0)
				ret = parse_command(c->cmd2, level + 1, c);
		}

		break;

	case OP_PIPE:
		/* redirect the output of the first command to the
		 * input of the second
		 */
		if (c->scmd == NULL && c->cmd1 != NULL && c->cmd2 != NULL)
			ret = do_on_pipe(c->cmd1, c->cmd2, level + 1, c);

		break;

	default:
		/* exit the shell */
		return SHELL_EXIT;
	}

	return ret; /* return actual exit code of command */
}
