#include "parse-config.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

void test_contains() {
    char *array[] = {"key", "value", NULL};
    assert(contains(array, 2, "key") == 1);
    assert(contains(array, 2, "ke") == 0); // This will likely fail currently
    printf("test_contains passed (if it didn't abort)\n");
}

void test_find_config_item() {
    config_t config[2];
    char *v1[] = {"key", "val", NULL};
    char *v2[] = {"key_extra", "val2", NULL};
    config[0].values = v1;
    config[0].value_count = 2;
    config[1].values = v2;
    config[1].value_count = 2;

    config_t *found = find_config_item(config, "key", 2);
    assert(found == &config[0]);
    
    config_t *found2 = find_config_item(config, "key_extra", 2);
    assert(found2 == &config[1]);

    config_t *found3 = find_config_item(config, "ke", 2);
    assert(found3 == NULL);
    printf("test_find_config_item passed\n");
}

int main() {
    test_contains();
    test_find_config_item();
    printf("All tests passed!\n");
    return 0;
}
