// SPDX-FileCopyrightText: Copyright 2025-2026 shadLauncher4 Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <filesystem>
#include <string>
#include <vector>
#include "core/file_format/pkg.h"
#include "core/file_format/psf.h"
#include "pkg_installer/pkg_installer.h"

std::optional<std::string> ReadPkgMetadata(const std::filesystem::path& pkg_path,
                                           PkgMetadata& out) try {
    if (!std::filesystem::exists(pkg_path)) {
        return std::string("PKG file does not exist: ") + pkg_path.string();
    }
    PKG pkg;
    std::string failReason;
    if (!pkg.Open(pkg_path, failReason)) {
        return failReason;
    }
    out.title_id = pkg.GetTitleID();
    out.pkg_size = pkg.GetPkgSize();
    out.flags = pkg.GetPkgHeader().pkg_content_flags; // GetPkgFlags returns string, we want numeric
    out.file_count = pkg.GetNumberOfFiles();
    PSF psf;
    if (psf.Open(pkg.sfo)) {
        out.category = std::string{psf.GetString("CATEGORY").value_or("")};
        out.content_id = std::string{psf.GetString("CONTENT_ID").value_or("")};
    }
    return std::nullopt;
} catch (const std::exception& e) {
    return std::string("Failed to read PKG: ") + e.what();
}

std::optional<std::string> ExtractPkg(const std::filesystem::path& pkg_path,
                                      const std::filesystem::path& out_dir,
                                      const std::vector<int>& indices,
                                      const ProgressCallback& progress_cb) try {
    if (!std::filesystem::exists(pkg_path)) {
        return std::string("PKG file does not exist: ") + pkg_path.string();
    }

    PKG pkg;
    std::string failReason;
    if (!pkg.Open(pkg_path, failReason)) {
        return failReason;
    }

    std::filesystem::create_directories(out_dir);

    if (!pkg.Extract(pkg_path, out_dir, failReason)) {
        return failReason;
    }

    uint32_t total = pkg.GetNumberOfFiles();
    std::vector<int> list = indices;
    if (list.empty()) {
        list.reserve(total);
        for (uint32_t i = 0; i < total; ++i)
            list.push_back(static_cast<int>(i));
    }

    uint32_t extracted = 0;
    for (int idx : list) {
        try {
            pkg.ExtractFiles(idx);
        } catch (const std::exception& e) {
            return std::string("Failed extracting file index ") + std::to_string(idx) + ": " +
                   e.what();
        } catch (...) {
            return std::string("Unknown error extracting file index ") + std::to_string(idx);
        }
        ++extracted;
        if (progress_cb)
            progress_cb(extracted, list.size());
    }
    return std::nullopt;
} catch (const std::exception& e) {
    // For example a folder that cannot be created (access denied, path too long).
    return std::string("Failed to extract PKG: ") + e.what();
}

std::vector<std::filesystem::path> ListPkgFiles(const std::filesystem::path& dir, bool recursive) {
    std::vector<std::filesystem::path> out;
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
        return out;
    if (recursive) {
        for (auto& e : std::filesystem::recursive_directory_iterator(dir)) {
            if (e.is_regular_file() && e.path().extension() == ".pkg")
                out.push_back(e.path());
        }
    } else {
        for (auto& e : std::filesystem::directory_iterator(dir)) {
            if (e.is_regular_file() && e.path().extension() == ".pkg")
                out.push_back(e.path());
        }
    }
    return out;
}
