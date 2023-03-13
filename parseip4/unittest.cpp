#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>

#include "common.h"
#include "naive.cpp"
#include "sse.cpp"
#include "glibc_ref.cpp"

#include <immintrin.h>

struct testcase {
    std::string ipv4;
    int err;
    std::string name;
};

bool same_errors(int err1, int err2) {
    if (err1 == err2) {
        return true;
    }

    if (err1 == errInvalidInput || err2 == errInvalidInput) {
        return true;
    }

    return false;
}


template <typename T>
bool test_wrong_input(T procedure) {
    std::vector<testcase> testcases = {
        {"ip", errTooShort, "string too short"},
        {"123.123.123.124    ", errTooLong, "string too long"},
        {"a.0.0.0", errWrongCharacter, "not a digit (1)"},
        {"0.z.0.0", errWrongCharacter, "not a digit (2)"},
        {"0.0.?.0", errWrongCharacter, "not a digit (3)"},
        {"0.0.0.%", errWrongCharacter, "not a digit (4)"},
        {"1.2.3.4.5", errTooManyFields, "too many dots"},
        {"192.168.10", errTooFewFields, "too few dots"},
        {"1.2.3.400", errTooBig, "too big number"},
        {"192.2..4", errEmptyField, "too few chars"},
        {"1.2.11111.4", errTooManyDigits, "too many chars"},

        // testcases copied from Go: src/net/netip/netip_test.go
        {"", errInvalidInput, "Empty string"},
        {"bad", errInvalidInput, "Garbage non-IP"},
        {"1234", errInvalidInput, "Single number"},
        {"1.2.3.4%eth0", errInvalidInput, "IPv4 with a zone specifier"},
        {".1.2.3", errInvalidInput, "IPv4 field must have at least one digit (1)"},
        {"1.2.3", errInvalidInput, "IPv4 field must have at least one digit (2)"},
        {"1..2.3", errInvalidInput, "IPv4 field must have at least one digit (3)"},
        {"1.2.3.4.5", errInvalidInput, "IPv4 address too long"},
		{"0300.0250.0214.0377", errInvalidInput, "IPv4 in dotted octal form"},
        {"0xc0.0xa8.0x8c.0xff", errInvalidInput, "IPv4 in dotted hex form"},
		{"192.168.12345", errInvalidInput, "IPv4 in class B form (1)"},
        {"127.0.1", errInvalidInput, "IPv4 in class B form (2)"},
        {"192.1234567", errInvalidInput, "IPv4 in class A form (1)"},
        {"127.1", errInvalidInput, "IPv4 in class A form (2)"},
        {"192.168.300.1", errInvalidInput, "IPv4 field has value >255"},
        {"192.168.0.1.5.6", errInvalidInput, "IPv4 with too many fields"}
    };

    for (const auto& tc: testcases) {
        const auto res = procedure(tc.ipv4);
        if (!same_errors(res.err, tc.err)) {
            printf("got : %d\n", res.err);
            printf("want: %d\n", tc.err);
            printf("%s: wrong result\n", tc.name.c_str());
            assert(false);
        }
    }

    return true;
}

std::string ipv4str(uint32_t x) {
    char tmp[16];
    sprintf(tmp, "%d.%d.%d.%d", (x >> 24), (x >> 16) & 0xff, (x >> 8) & 0xff, x & 0xff);

    return tmp;
}


template <typename T>
bool test_valid_inputs(T procedure) {
    int i = 0;
    for (uint64_t tmp = 0; tmp <= 0xffffffff; tmp += 1031) {
        uint32_t ipv4 = tmp;
        const std::string img = ipv4str(ipv4);

        const auto res = procedure(img);
        if (res.err != 0) {
            printf("IPv4: %s\n", img.c_str());
            printf("hex : %08x\n", ipv4);
            printf("classified as invalid: err code=%d\n", res.err);
            return false;
        }

        if (res.ipv4 != uint32_t(ipv4)) {
            printf("IPv4: %s\n", img.c_str());
            printf("got : %08x\n", res.ipv4);
            printf("want: %08x\n", ipv4);
            printf("wrongly parsed\n");
            return false;
        }

        i++;
    }

    return true;
}

int8_t get(__m128i x) {
    return _mm_cvtsi128_si32(x) & 0xff;
}

int main() {
    srand(0);

    bool ok = true;

    puts("glibc");
    ok = test_wrong_input(glibc_parse_ipv4) && ok;
    ok = test_valid_inputs(glibc_parse_ipv4) && ok;

    puts("naive");
    ok = test_wrong_input(naive_parse_ipv4) && ok;
    ok = test_valid_inputs(naive_parse_ipv4) && ok;

    puts("naive (no validation)");
    ok = test_valid_inputs(naive_parse_ipv4_no_validation) && ok;

    puts("SSE");
    ok = test_wrong_input(sse_parse_ipv4) && ok;
    ok = test_valid_inputs(sse_parse_ipv4) && ok;

    puts("SSE (v2)");
    ok = test_wrong_input(sse_parse_ipv4_v2) && ok;
    ok = test_valid_inputs(sse_parse_ipv4_v2) && ok;

    puts("SSE (v3)");
    ok = test_valid_inputs(sse_parse_ipv4_v3) && ok;

    puts("SSE (v4)");
    ok = test_valid_inputs(sse_parse_ipv4_v4) && ok;

    puts("SSE (v5)");
    ok = test_valid_inputs(sse_parse_ipv4_v5) && ok;

    puts("SSE (v6)");
    ok = test_valid_inputs(sse_parse_ipv4_v6) && ok;

    puts("SSE (v7)");
    ok = test_valid_inputs(sse_parse_ipv4_v7) && ok;

    if (ok) {
        puts("All OK");
    }
}
