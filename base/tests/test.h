#pragma once

typedef struct {
    char const* title;
    I32 tests_count;
    I32 failed_count;
} TestCase;

static inline
bool test_expr(TestCase* t, bool predicate, char const* msg, I32 line, char const* func){
    t->tests_count += 1;
    if(!predicate){
        t->failed_count += 1;
        printf("  (%s:%d) failed: %s\n", func, line, msg);
    }
    return predicate;
}

static inline
TestCase test_begin(char const * title){
    printf("[\e[1;37m%s\e[0m]\n", title);
    TestCase t = {0};
    t.title = title;
    return t;
}

static inline
void test_end(TestCase t){
    printf("%s ok in %d / %d\n",
        t.failed_count != 0 ? "\e[31mFAIL\e[0m" : "\e[32mPASS\e[0m",
        t.tests_count - t.failed_count,
        t.tests_count
    );
}

#define Test(Pred) test_expr(&_test, (Pred), #Pred, __LINE__, __func__)
#define TEST_BEGIN(Title) TestCase _test  = test_begin(Title);
#define TEST_END test_end(_test)