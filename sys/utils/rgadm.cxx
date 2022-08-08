/* rgadm.c
 *
 * Registry Administrator
 * 
 * Implements a BTree+ registry manager. A registry is basically a big control
 * panel which has a key and a value, when a value is written the call is piped
 * to a driver in the kernel space in order to perform an action.
 * 
 * A practical usage would be a NVMe driver which has a SubsystemId in the
 * registry HLOCAL_DRIVES_NVME0_SSID, if the hypotetical NVMe driver could
 * change the SubsystemId of a drive then a write operation on the registry
 * would make the NVMe drive evaluate the new SSID and reflect the changes
 * via hardware procedures.
 * 
 * In short, the registry manager is an alternative form of a virtual filesystem
 * designed specifically for dynamic assignment (values can be strings, numbers,
 * etcetera). Something that is commonly ignored on VFS implementations which
 * can be useful for driver developers.
 */

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>

/* The registry manager serves as a hierachy of environment variables where
 * drivers can place their own registers as applications can too
 *
 * The purpouse is to redirect writes/reads to an enviroment variable by doing a
 * remote call to an application or driver */

#define MAX_REGISTRY_PATH 255

#define MAX_REGISTRY_NAME 16
#define MAX_REGISTRY_KEY 16

struct registry_group {
    char *name;

    struct registry_group *groups;
    size_t n_groups;

    struct registry_key *keys;
    size_t n_keys;
};

struct registry_key {
    char *name;

    void *data;
    size_t n_data;

    void (*read)(void *buffer, size_t len);
    void (*write)(const void *buffer, size_t len);
};

void registry_init(void);
registry_group *registry_get_root_group(void);
registry_group *registry_group_create(registry_group *root, const char *name);
registry_key *registry_resolve_path(registry_group *root, const char *path);
registry_group *registry_group_add_group(registry_group *root, registry_group *subgroup);
registry_key *registry_group_add_key(registry_group *root, registry_key *key);
registry_key *registry_group_find_key(const registry_group *root, const char *name);
registry_group *registry_group_find_group(const registry_group *root, const char *name);
void registry_group_delete(registry_group *group);
registry_key *registry_key_create(registry_group *root, const char *name);
void registry_key_delete(registry_key *key);

void registry_key_dump(const registry_key *key, int level);
void registry_group_dump(const registry_group *root, int level);

/** @todo Initialize root group to 0 */
struct registry_group *g_root_group;

void registry_init(void)
{
    g_root_group = registry_group_create(nullptr, "HKEY");
    return;
}

struct registry_group *registry_get_root_group(void)
{
    return g_root_group;
}

struct registry_group *registry_group_create(struct registry_group *root, const char *name)
{
    assert(name != nullptr);
    size_t len = strlen(name);
    assert(len < MAX_REGISTRY_NAME && len != 0);

    auto *group = (registry_group *)calloc(1, sizeof(struct registry_group));
    if(group == nullptr) {
        printf("Out of memory\r\n");
        return nullptr;
    }

    group->name = (char *)malloc(len + 1);
    if(group->name == nullptr) {
        printf("Out of memory\r\n");
        free(group);
        return nullptr;
    }
    strcpy(group->name, name);

    if(root != nullptr) {
        group = registry_group_add_group(root, group);
    }
    return group;
}

struct registry_key *registry_resolve_path(struct registry_group *root, const char *path)
{
    assert(root != nullptr && path != nullptr);

    size_t len = strlen(path);
    if(len >= MAX_REGISTRY_PATH) {
        printf("Length exceeds max registry path\r\n");
        return nullptr;
    }

    if(len == 0) {
        printf("Zero-length registry path\r\n");
        return nullptr;
    }

    char *tmpbuf = (char *)malloc(len + 1);
    if(tmpbuf == nullptr) {
        printf("Out of memory\r\n");
        return nullptr;
    }
    strcpy(tmpbuf, path);

    const struct registry_group *group = root;
    const struct registry_key *key = nullptr;
    char *buf = tmpbuf;
    while(*buf != '\0') {
        const struct registry_group *old_group;
        const char *old_tmpbuf;
        size_t name_len;

        /* Find the next separator, these separators allow for namespaces
         * which are then parsed into groups -> keys */
        old_tmpbuf = buf;
        buf = strpbrk(buf, "_-:");
        if(buf == nullptr) {
            /* ifno separators were found it means we are at the key */
            name_len = strlen(old_tmpbuf);
        } else {
            name_len = (size_t)((ptrdiff_t)buf - (ptrdiff_t)old_tmpbuf);
            if(!name_len) {
                printf("Zero-lenght name");
                key = nullptr;
                goto end;
            }
        }

        /* Skip the separator itself */
        buf++;

        char *name = strndup(old_tmpbuf, name_len);
        if(name == nullptr) {
            printf("Ouf of memory");
            key = nullptr;
            goto end;
        }

        old_group = group;
        group = registry_group_find_group(group, name);
        if(group == nullptr) {
            group = old_group;
            key = registry_group_find_key(group, name);
            if(key == nullptr) {
                printf("Registry %s not found", name);
                goto end;
            }
            free(name);
            break;
        }
        free(name);
    }
end:
    free(tmpbuf);
    return (struct registry_key *)key;
}

registry_group *registry_group_add_group(registry_group *root, registry_group *subgroup)
{
    assert(root != nullptr && subgroup != nullptr);

    root->groups = (registry_group *)realloc(root->groups, (root->n_groups + 1) * sizeof(registry_group));
    if(root->groups == nullptr) {
        printf("Out of memory");
        return nullptr;
    }
    root->groups[root->n_groups++] = *subgroup;
    return &root->groups[root->n_groups - 1];
}

registry_key *registry_group_add_key(registry_group *root, registry_key *key)
{
    assert(root != nullptr && key != nullptr);

    root->keys = (registry_key *)realloc(root->keys, (root->n_keys + 1) * sizeof(registry_key));
    if(root->keys == nullptr) {
        printf("Out of memory");
        return nullptr;
    }
    root->keys[root->n_keys++] = *key;
    return &root->keys[root->n_keys - 1];
}

registry_group *registry_group_find_group(const registry_group *root, const char *name)
{
    assert(root != nullptr && name != nullptr);

    if(strlen(name) >= MAX_REGISTRY_KEY) {
        printf("Length exceeds max registry key");
        return nullptr;
    }

    for(size_t i = 0; i < root->n_groups; i++) {
        auto *subgroup = &root->groups[i];
        if(!strcmp(subgroup->name, name)) {
            return subgroup;
        }
    }
    return nullptr;
}

registry_key *registry_group_find_key(const registry_group *root, const char *name)
{
    assert(root != nullptr && name != nullptr);

    if(strlen(name) >= MAX_REGISTRY_NAME) {
        printf("Length exceeds max registry name");
        return nullptr;
    }

    for(size_t i = 0; i < root->n_keys; i++) {
        const auto *key = &root->keys[i];
        assert(key->name != nullptr);
        if(!strcmp(key->name, name)) {
            return (registry_key *)key;
        }
    }
    return nullptr;
}

void registry_group_delete(registry_group *group)
{
    free(group->name);
    free(group);
}

registry_key *registry_key_create(registry_group *root, const char *name)
{
    assert(root != nullptr && name != nullptr);
    if(strlen(name) >= MAX_REGISTRY_KEY) {
        printf("Length exceeds max registry key");
        return nullptr;
    }

    auto *key = (registry_key *)calloc(1, sizeof(struct registry_key));
    if(key == nullptr) {
        printf("Out of memory");
        return nullptr;
    }

    key->name = strdup(name);
    if(key->name == nullptr) {
        printf("Out of memory");
        free(key);
        return nullptr;
    }

    if(root != nullptr) {
        struct registry_key *old_key = key;
        key = registry_group_add_key(root, old_key);
        if(key == nullptr) {
            registry_key_delete(old_key);
        }
    }
    return key;
}

void registry_key_delete(struct registry_key *key)
{
    assert(key != nullptr);
    free(key->name);
    free(key);
}

void registry_key_dump(const struct registry_key *key, int level)
{
    assert(key != nullptr);
    for(size_t i = 0; i < (size_t)level; i++)
        dprintf("    ");
    dprintf("[HKEY] %s", key->name);
}

void registry_group_dump(const struct registry_group *root, int level)
{
    assert(root != nullptr);
    for(size_t i = 0; i < (size_t)level; i++)
        dprintf("    ");
    dprintf("[HGROUP] %s", root->name);

    for(size_t i = 0; i < root->n_groups; i++)
        registry_group_dump(&root->groups[i], level + 1);
    
    for(size_t i = 0; i < root->n_keys; i++)
        registry_key_dump(&root->keys[i], level + 1);
}

#define VERSION_STRING "v1.0"

int main(int argc, char **argv)
{
    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "/VERSION")) {
            printf("REGADM Daemon " VERSION_STRING "\r\n");
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr, "Unknown option %s\r\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    /* Initialize the registry (relational database of k=v pairs) engine */
    printf("Initializing the registry key manager\r\n");
    registry_init();
    registry_group_create(registry_get_root_group(), "HSYSTEM");
    registry_group_create(registry_get_root_group(), "HLOCAL");

    /** @todo Run as daemon */
    while(1) {

    }
    exit(EXIT_SUCCESS);
    return 0;
}
