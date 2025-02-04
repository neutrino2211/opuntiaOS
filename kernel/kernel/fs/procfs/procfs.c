/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/procfs/procfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>

// #define PROCFS_DEBUG

/**
 * ProcFS works with caches differently. It does NOT allow reading
 * of random inode in it's current implementation. Instead of
 * accessing inode randomly, every cache entry (dentry) will be
 * filled up with inode by lookup functions, and the ONLY way when
 * procfs_read_inode will be called is to get the root inode.
 */
extern const file_ops_t procfs_root_ops;
int procfs_read_inode(dentry_t* dentry)
{
    if (dentry->inode_indx != 2) {
        ASSERT("NOT ROOT ENTRY ID READ IN PROCFS");
    }
    procfs_inode_t* procfs_inode = (procfs_inode_t*)dentry->inode;
    memset((void*)procfs_inode, 0, sizeof(procfs_inode_t));
    procfs_inode->index = 2;
    procfs_inode->mode = S_IFDIR | 0444;
    procfs_inode->ops = &procfs_root_ops;
    return 0;
}

int procfs_write_inode(dentry_t* dentry)
{
    return 0;
}

int procfs_free_inode(dentry_t* dentry)
{
    return 0;
}

fsdata_t procfs_data(dentry_t* dentry)
{
    fsdata_t fsdata;
    fsdata.sb = NULL;
    fsdata.gt = NULL;
    fsdata.blksize = 1;
    return fsdata;
}

int procfs_can_write(file_t* file, size_t start)
{
    procfs_inode_t* procfs_inode = (procfs_inode_t*)file_dentry_assert(file)->inode;
    if (!procfs_inode->ops->can_write) {
        return -ENOEXEC;
    }
    return procfs_inode->ops->can_write(file, start);
}

int procfs_write(file_t* file, void __user* buf, size_t start, size_t len)
{
    procfs_inode_t* procfs_inode = (procfs_inode_t*)file_dentry_assert(file)->inode;
    if (!procfs_inode->ops->write) {
        return -ENOEXEC;
    }
    return procfs_inode->ops->write(file, buf, start, len);
}

int procfs_can_read(file_t* file, size_t start)
{
    procfs_inode_t* procfs_inode = (procfs_inode_t*)file_dentry_assert(file)->inode;
    if (!procfs_inode->ops->can_read) {
        return -ENOEXEC;
    }
    return procfs_inode->ops->can_read(file, start);
}

int procfs_read(file_t* file, void __user* buf, size_t start, size_t len)
{
    procfs_inode_t* procfs_inode = (procfs_inode_t*)file_dentry_assert(file)->inode;
    if (!procfs_inode->ops->read) {
        return -ENOEXEC;
    }
    return procfs_inode->ops->read(file, buf, start, len);
}

int procfs_getdents(dentry_t* dir, void __user* buf, off_t* offset, size_t len)
{
    procfs_inode_t* procfs_inode = (procfs_inode_t*)dir->inode;
    if (!procfs_inode->ops->getdents) {
        return -ENOEXEC;
    }
    return procfs_inode->ops->getdents(dir, buf, offset, len);
}

int procfs_lookup(const path_t* path, const char* name, size_t len, path_t* result)
{
    procfs_inode_t* procfs_inode = (procfs_inode_t*)path->dentry->inode;
    if (!procfs_inode->ops->lookup) {
        return -ENOEXEC;
    }
    return procfs_inode->ops->lookup(path, name, len, result);
}

driver_desc_t _procfs_driver_info()
{
    driver_desc_t fs_desc = { 0 };
    fs_desc.type = DRIVER_FILE_SYSTEM;
    fs_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_PREPARE_FS] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_OPEN] = NULL; /* No custom open, vfs will use its code */
    fs_desc.functions[DRIVER_FILE_SYSTEM_CAN_READ] = procfs_can_read;
    fs_desc.functions[DRIVER_FILE_SYSTEM_READ] = procfs_read;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CAN_WRITE] = procfs_can_write;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE] = procfs_write;
    fs_desc.functions[DRIVER_FILE_SYSTEM_TRUNCATE] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_MKDIR] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE] = NULL;

    fs_desc.functions[DRIVER_FILE_SYSTEM_READ_INODE] = procfs_read_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE_INODE] = procfs_write_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_FREE_INODE] = procfs_free_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GET_FSDATA] = procfs_data;
    fs_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP] = procfs_lookup;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GETDENTS] = procfs_getdents;

    fs_desc.functions[DRIVER_FILE_SYSTEM_FSTAT] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_FCHMOD] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_IOCTL] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_MMAP] = NULL;
    return fs_desc;
}

void procfs_install()
{
    devman_register_driver(_procfs_driver_info(), "procfs");
}
devman_register_driver_installation(procfs_install);

int procfs_mount()
{
    path_t vfspth;
    if (vfs_resolve_path("/proc", &vfspth) < 0) {
        return -ENOENT;
    }
    int driver_id = vfs_get_fs_id("procfs");
    if (driver_id < 0) {
#ifdef PROCFS_DEBUG
        log("Procfs: no driver is installed, exiting");
#endif
        return -ENOENT;
    }
#ifdef PROCFS_DEBUG
    log("procfs: %x", driver_id);
#endif
    int err = vfs_mount(&vfspth, new_virtual_device(DEVICE_STORAGE), driver_id);
    path_put(&vfspth);
    return err;
}