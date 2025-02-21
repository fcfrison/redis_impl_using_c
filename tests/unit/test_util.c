#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/wait.h>
#include "../include/util.h"

// Test case 1: No arguments provided, use default values
void test_cli_parser_no_arguments() {
    char* dir = NULL;
    char* dbfilename = NULL;
    char* default_redis_dir = "/default/dir";
    char* default_redis_rdbfile = "dump.rdb";

    int argc = 1;
    char* argv[] = {"./program"};
    optind = 1; 
    cli_parser(argc, argv, &dir, &dbfilename, default_redis_dir, default_redis_rdbfile);

    assert(strcmp(dir, default_redis_dir) == 0);
    assert(strcmp(dbfilename, default_redis_rdbfile) == 0);

    free(dir);
    free(dbfilename);
}
// Test case 2: Provide --dir argument
void test_cli_parser_with_dir_argument() {
    char* dir = NULL;
    char* dbfilename = NULL;
    char* default_redis_dir = "/default/dir";
    char* default_redis_rdbfile = "dump.rdb";

    int argc = 3;
    char* argv[] = {"./program", "--dir", "/custom/dir"};
    optind = 1; 
    cli_parser(argc, argv, &dir, &dbfilename, default_redis_dir, default_redis_rdbfile);

    assert(strcmp(dir, "/custom/dir") == 0);
    assert(strcmp(dbfilename, default_redis_rdbfile) == 0);

    free(dbfilename);
}
// Test case 3: Provide --dbfilename argument
void test_cli_parser_with_dbfilename_argument() {
    char* dir = NULL;
    char* dbfilename = NULL;
    char* default_redis_dir = "/default/dir";
    char* default_redis_rdbfile = "dump.rdb";

    int argc = 3;
    char* argv[] = {"./program", "--dbfilename", "custom.rdb"};
    
    cli_parser(argc, argv, &dir, &dbfilename, default_redis_dir, default_redis_rdbfile);

    assert(strcmp(dir, default_redis_dir) == 0);
    assert(strcmp(dbfilename, "custom.rdb") == 0);

    free(dir);
}
// Test case 4: Provide both --dir and --dbfilename arguments
void test_cli_parser_with_both_arguments() {
    char* dir = NULL;
    char* dbfilename = NULL;
    char* default_redis_dir = "/default/dir";
    char* default_redis_rdbfile = "dump.rdb";

    int argc = 5;
    char* argv[] = {"./program", "--dir", "/custom/dir", "--dbfilename", "custom.rdb"};

    cli_parser(argc, argv, &dir, &dbfilename, default_redis_dir, default_redis_rdbfile);

    assert(strcmp(dir, "/custom/dir") == 0);
    assert(strcmp(dbfilename, "custom.rdb") == 0);
}
// Test case 5: Provide invalid argument (should exit with failure)
void test_cli_parser_invalid_argument() {
    char* dir = NULL;
    char* dbfilename = NULL;
    char* default_redis_dir = "/default/dir";
    char* default_redis_rdbfile = "dump.rdb";

    int argc = 2;
    char* argv[] = {"./program", "--invalid"};

    // Redirect stderr to avoid printing error messages during the test
    freopen("/dev/null", "w", stderr);

    // Expect the program to exit with failure
    if (fork() == 0) {
        cli_parser(argc, argv, &dir, &dbfilename, default_redis_dir, default_redis_rdbfile);
        exit(0); // If it doesn't exit, this will fail the test
    } else {
        int status;
        wait(&status);
        assert(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE);
    }

    // Restore stderr
    freopen("/dev/tty", "w", stderr);
}
// Test case 6: Provide --dir argument without value (should exit with failure)
void test_cli_parser_missing_dir_value() {
    char* dir = NULL;
    char* dbfilename = NULL;
    char* default_redis_dir = "/default/dir";
    char* default_redis_rdbfile = "dump.rdb";

    int argc = 2;
    char* argv[] = {"./program", "--dir"};

    // Redirect stderr to avoid printing error messages during the test
    freopen("/dev/null", "w", stderr);

    // Expect the program to exit with failure
    if (fork() == 0) {
        cli_parser(argc, argv, &dir, &dbfilename, default_redis_dir, default_redis_rdbfile);
        exit(0); // If it doesn't exit, this will fail the test
    } else {
        int status;
        wait(&status);
        assert(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE);
    }

    // Restore stderr
    freopen("/dev/tty", "w", stderr);
}

// Test case 7: Provide --dbfilename argument without value (should exit with failure)
void test_cli_parser_missing_dbfilename_value() {
    char* dir = NULL;
    char* dbfilename = NULL;
    char* default_redis_dir = "/default/dir";
    char* default_redis_rdbfile = "dump.rdb";

    int argc = 2;
    char* argv[] = {"./program", "--dbfilename"};

    // Redirect stderr to avoid printing error messages during the test
    freopen("/dev/null", "w", stderr);

    // Expect the program to exit with failure
    if (fork() == 0) {
        cli_parser(argc, argv, &dir, &dbfilename, default_redis_dir, default_redis_rdbfile);
        exit(0); // If it doesn't exit, this will fail the test
    } else {
        int status;
        wait(&status);
        assert(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE);
    }

    // Restore stderr
    freopen("/dev/tty", "w", stderr);
}
int main() {
    test_cli_parser_no_arguments();
    test_cli_parser_with_dir_argument();
    test_cli_parser_with_dbfilename_argument();
    test_cli_parser_with_both_arguments();
    test_cli_parser_invalid_argument();
    test_cli_parser_missing_dbfilename_value();
    printf("All tests passed!\n");
    return 0;
}