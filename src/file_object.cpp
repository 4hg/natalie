#include "natalie.hpp"

#include <errno.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/stat.h>

namespace Natalie {

// If it's not a string but has a to_path method then execute that method.
// make sure the path or to_path result is a String before continuing.
// this is common to many functions probably belongs somewhere else
Value ConvertToPath(Env *env, Value path) {
    if (!path->is_string() && path->respond_to(env, "to_path"_s))
        path = path->send(env, "to_path"_s, { path });
    path->assert_type(env, Object::Type::String, "String");
    return path;
}

Value FileObject::initialize(Env *env, Value filename, Value flags_obj, Block *block) {
    filename->assert_type(env, Object::Type::String, "String");
    int flags = O_RDONLY;
    if (flags_obj) {
        switch (flags_obj->type()) {
        case Object::Type::Integer:
            flags = flags_obj->as_integer()->to_nat_int_t();
            break;
        case Object::Type::String: {
            const char *flags_str = flags_obj->as_string()->c_str();
            if (strcmp(flags_str, "r") == 0) {
                flags = O_RDONLY;
            } else if (strcmp(flags_str, "r+") == 0) {
                flags = O_RDWR;
            } else if (strcmp(flags_str, "w") == 0) {
                flags = O_WRONLY | O_CREAT | O_TRUNC;
            } else if (strcmp(flags_str, "w+") == 0) {
                flags = O_RDWR | O_CREAT | O_TRUNC;
            } else if (strcmp(flags_str, "a") == 0) {
                flags = O_WRONLY | O_CREAT | O_APPEND;
            } else if (strcmp(flags_str, "a+") == 0) {
                flags = O_RDWR | O_CREAT | O_APPEND;
            } else {
                env->raise("ArgumentError", "invalid access mode {}", flags_str);
            }
            break;
        }
        default:
            env->raise("TypeError", "no implicit conversion of {} into String", flags_obj->klass()->inspect_str());
        }
    }
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    int fileno = ::open(filename->as_string()->c_str(), flags, mode);
    if (fileno == -1) {
        env->raise_errno();
    } else {
        set_fileno(fileno);
        return this;
    }
}

Value FileObject::expand_path(Env *env, Value path, Value root) {
    path->assert_type(env, Object::Type::String, "String");
    StringObject *merged;
    if (path->as_string()->length() > 0 && path->as_string()->c_str()[0] == '/') {
        merged = path->as_string();
    } else if (root) {
        root->assert_type(env, Object::Type::String, "String");
        root = expand_path(env, root, nullptr);
        merged = StringObject::format("{}/{}", root->as_string(), path->as_string());
    } else {
        char root[MAXPATHLEN + 1];
        if (!getcwd(root, MAXPATHLEN + 1))
            env->raise_errno();
        merged = StringObject::format("{}/{}", root, path->as_string());
    }
    // collapse ..
    RegexpObject dotdot { env, "[^/]*/\\.\\.(/|\\z)" };
    StringObject empty_string { "" };
    do {
        merged = merged->sub(env, &dotdot, &empty_string)->as_string();
    } while (env->has_last_match());
    // collapse .
    RegexpObject dot { env, "/\\.(/|\\z)" };
    StringObject slash { "/" };
    do {
        merged = merged->sub(env, &dot, &slash)->as_string();
    } while (env->has_last_match());
    // remove trailing slash
    if (merged->length() > 1 && merged->c_str()[merged->length() - 1] == '/') {
        merged->truncate(merged->length() - 1);
    }
    return merged;
}

// TODO: Accept variable arguments, return value is number of args instead of 1.
Value FileObject::unlink(Env *env, Value path) {
    path = ConvertToPath(env, path);
    int result = ::unlink(path->as_string()->c_str());
    if (result == 0) {
        return Value::integer(1);
    } else {
        env->raise_errno();
    }
}

void FileObject::build_constants(Env *env, ClassObject *klass) {
    Value Constants = new ModuleObject { "Constants" };
    klass->const_set("Constants"_s, Constants);
    klass->const_set("APPEND"_s, Value::integer(O_APPEND));
    Constants->const_set("APPEND"_s, Value::integer(O_APPEND));
    klass->const_set("RDONLY"_s, Value::integer(O_RDONLY));
    Constants->const_set("RDONLY"_s, Value::integer(O_RDONLY));
    klass->const_set("WRONLY"_s, Value::integer(O_WRONLY));
    Constants->const_set("WRONLY"_s, Value::integer(O_WRONLY));
    klass->const_set("TRUNC"_s, Value::integer(O_TRUNC));
    Constants->const_set("TRUNC"_s, Value::integer(O_TRUNC));
    klass->const_set("CREAT"_s, Value::integer(O_CREAT));
    Constants->const_set("CREAT"_s, Value::integer(O_CREAT));
    klass->const_set("DSYNC"_s, Value::integer(O_DSYNC));
    Constants->const_set("DSYNC"_s, Value::integer(O_DSYNC));
    klass->const_set("EXCL"_s, Value::integer(O_EXCL));
    Constants->const_set("EXCL"_s, Value::integer(O_EXCL));
    klass->const_set("NOCTTY"_s, Value::integer(O_NOCTTY));
    Constants->const_set("NOCTTY"_s, Value::integer(O_NOCTTY));
    klass->const_set("NOFOLLOW"_s, Value::integer(O_NOFOLLOW));
    Constants->const_set("NOFOLLOW"_s, Value::integer(O_NOFOLLOW));
    klass->const_set("NONBLOCK"_s, Value::integer(O_NONBLOCK));
    Constants->const_set("NONBLOCK"_s, Value::integer(O_NONBLOCK));
    klass->const_set("RDWR"_s, Value::integer(O_RDWR));
    Constants->const_set("RDWR"_s, Value::integer(O_RDWR));
    klass->const_set("SYNC"_s, Value::integer(O_SYNC));
    Constants->const_set("SYNC"_s, Value::integer(O_SYNC));

    klass->const_set("LOCK_EX"_s, Value::integer(LOCK_EX));
    Constants->const_set("LOCK_EX"_s, Value::integer(LOCK_EX));
    klass->const_set("LOCK_NB"_s, Value::integer(LOCK_NB));
    Constants->const_set("LOCK_NB"_s, Value::integer(LOCK_NB));
    klass->const_set("LOCK_SH"_s, Value::integer(LOCK_SH));
    Constants->const_set("LOCK_SH"_s, Value::integer(LOCK_SH));
    klass->const_set("LOCK_UN"_s, Value::integer(LOCK_UN));
    Constants->const_set("LOCK_UN"_s, Value::integer(LOCK_UN));
}

bool FileObject::is_file(Env *env, Value path) {
    struct stat sb;
    path = ConvertToPath(env, path);
    if (stat(path->as_string()->c_str(), &sb) == -1)
        return false;
    return S_ISREG(sb.st_mode);
}

bool FileObject::is_directory(Env *env, Value path) {
    struct stat sb;
    path = ConvertToPath(env, path);
    if (stat(path->as_string()->c_str(), &sb) == -1)
        return false;
    return S_ISDIR(sb.st_mode);
}

bool FileObject::is_identical(Env *env, Value file1, Value file2) {
    file1 = ConvertToPath(env, file1);
    file2 = ConvertToPath(env, file2);
    struct stat stat1;
    struct stat stat2;
    auto result1 = ::stat(file1->as_string()->c_str(), &stat1);
    auto result2 = ::stat(file2->as_string()->c_str(), &stat2);
    if (result1 < 0) return false;
    if (result2 < 0) return false;
    if (stat1.st_dev != stat2.st_dev) return false;
    if (stat1.st_ino != stat2.st_ino) return false;
    return true;
}

bool FileObject::is_symlink(Env *env, Value path) {
    struct stat sb;
    path = ConvertToPath(env, path);
    if (lstat(path->as_string()->c_str(), &sb) == -1)
        return false;
    return S_ISLNK(sb.st_mode);
}

bool FileObject::is_pipe(Env *env, Value path) {
    struct stat sb;
    path->assert_type(env, Object::Type::String, "String");
    if (stat(path->as_string()->c_str(), &sb) == -1)
        return false;
    return S_ISFIFO(sb.st_mode);
}

bool FileObject::is_socket(Env *env, Value path) {
    struct stat sb;
    path->assert_type(env, Object::Type::String, "String");
    if (stat(path->as_string()->c_str(), &sb) == -1)
        return false;
    return S_ISSOCK(sb.st_mode);
}

bool FileObject::is_readable(Env *env, Value path) {
    path = ConvertToPath(env, path);
    if (access(path->as_string()->c_str(), R_OK) == -1)
        return false;
    return true;
}

bool FileObject::is_writable(Env *env, Value path) {
    path = ConvertToPath(env, path);
    if (access(path->as_string()->c_str(), W_OK) == -1)
        return false;
    return true;
}

bool FileObject::is_executable(Env *env, Value path) {
    path = ConvertToPath(env, path);
    if (access(path->as_string()->c_str(), X_OK) == -1)
        return false;
    return true;
}

bool FileObject::is_zero(Env *env, Value path) {
    struct stat sb;
    path = ConvertToPath(env, path);
    if (stat(path->as_string()->c_str(), &sb) == -1)
        return false;
    return (sb.st_size == 0);
}

Value FileObject::symlink(Env *env, Value from, Value to) {
    from = ConvertToPath(env, from);
    to = ConvertToPath(env, to);
    int result = ::symlink(from->as_string()->c_str(), to->as_string()->c_str());
    if (result < 0) env->raise_errno();
    return Value::integer(0);
}

Value FileObject::link(Env *env, Value from, Value to) {
    from = ConvertToPath(env, from);
    to = ConvertToPath(env, to);
    int result = ::link(from->as_string()->c_str(), to->as_string()->c_str());
    if (result < 0) env->raise_errno();
    return Value::integer(0);
}

// TODO: Handle mode properly
Value FileObject::mkfifo(Env *env, Value path, Value mode) {
    mode_t octmode = 0666;
    path = ConvertToPath(env, path);
    int result = ::mkfifo(path->as_string()->c_str(), octmode);
    if (result < 0) env->raise_errno();
    return Value::integer(0);
}

// TODO: chmod can take multiple paths, implement that later.
Value FileObject::chmod(Env *env, Value mode, Value path) {
    path = ConvertToPath(env, path);
    mode->assert_type(env, Object::Type::Integer, "Integer");
    mode_t modenum = IntegerObject::convert_to_int(env, mode);
    int result = ::chmod(path->as_string()->c_str(), modenum);
    if (result < 0) env->raise_errno();
    return Value::integer(1); // return # of files
}

}
