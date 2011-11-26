#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "fmt.h"
#include "seq.h"
#include "umlab.h"

extern void Um_write_sequence(FILE *output, Seq_T asm);

static struct test_info {
  const char *name;
  const char *test_input;          // NULL means no input needed
  const char *expected_output;
  void (*emit_test)(Seq_T stream); // writes instructions into sequence
} tests[] = {
  { "halt", NULL, "",               emit_halt_test },
  { "goto", NULL, "GOTO passed.\n", emit_goto_test },
  { "IO", "hello world!", "hello world!", emit_IO_test },
  { "add", NULL, "PASSED.", emit_add_test },
  { "multiply", NULL, "PASSED.", emit_multiply_test },
  { "divide", NULL, "PASSED.", emit_divide_test },
  { "move", NULL, "Conditional Move on zero register passed.\n",
                                emit_move_test },
  { "nand", NULL, "PASSED.", emit_NAND_test },
  { "map", NULL, "Map / unmap passed.", emit_mapUnmap_test },
  { "nomove", NULL, "hh", emit_nonMove_test },
  { "loadprog", NULL, "hi", emit_loadprog_test }
};
  
#define NTESTS (sizeof(tests)/sizeof(tests[0]))

static FILE *open_and_free_pathname(char *path);
// open file 'path' for writing, then free the pathname;
// if anything fails, checked runtime error

static void write_or_remove_file(char *path, const char *contents);
// if contents is NULL or empty, remove the given 'path', 
// otherwise write 'contents' into 'path'.  Either way, free 'path'.

static void write_test_files(struct test_info *test) {
  FILE *binary = open_and_free_pathname(Fmt_string("%s.um", test->name));
  Seq_T asm = Seq_new(0);
  test->emit_test(asm);
  Um_write_sequence(binary, asm);
  Seq_free(&asm);
  fclose(binary);

  write_or_remove_file(Fmt_string("%s.0", test->name), test->test_input);
  write_or_remove_file(Fmt_string("%s.1", test->name), test->expected_output);
}

int main (int argc, char *argv[]) {
  bool failed = false;
  if (argc == 1)
    for (unsigned i = 0; i < NTESTS; i++) {
      printf("***** Writing test '%s'.\n", tests[i].name);
      write_test_files(&tests[i]);
    }
  else
    for (int j = 1; j < argc; j++) {
      bool tested = false;
      for (unsigned i = 0; i < NTESTS; i++)
        if (!strcmp(tests[i].name, argv[j])) {
          tested = true;
          write_test_files(&tests[i]);
        }
      if (!tested) {
        failed = true;
        fprintf(stderr, "***** No test named %s *****\n", argv[j]);
      }
    }
  return failed; // failed nonzero == exit nonzero == failure
}


static void write_or_remove_file(char *path, const char *contents) {
  if (contents == NULL || *contents == '\0') {
    remove(path);
  } else {
    FILE *input = fopen(path, "wb");
    assert(input);
    fputs(contents, input);
    fclose(input);
  }
  free(path);
}

static FILE *open_and_free_pathname(char *path) {
  FILE *fp = fopen(path, "wb");
  assert(fp);
  free(path);
  return fp;
}
