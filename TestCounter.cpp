#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
using namespace doctest;

const int MIN_TESTS = 20;

int return_code = 0;

struct ReporterCounter: public ConsoleReporter {
    ReporterCounter(const ContextOptions& input_options)
            : ConsoleReporter(input_options) {}
    };

REGISTER_REPORTER("counter", 1, ReporterCounter);

int main(int argc, char** argv) {
    Context context;
    context.addFilter("reporters", "counter");
    context.run();
    return return_code;
}
