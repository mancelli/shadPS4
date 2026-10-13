// SPDX-FileCopyrightText: Copyright 2025-2026 shadLauncher4 Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

struct PkgMetadata {
    std::string title_id;
    uint64_t pkg_size = 0;
    uint32_t flags = 0;
    uint32_t file_count = 0;
    std::string category;   // param.sfo CATEGORY: "gd" game, "gp" patch, "ac" additional content
    std::string content_id; // param.sfo CONTENT_ID, e.g. UP0006-CUSA14204_00-ENTITLEMENTLABEL0
};

// Bytes of file data written so far and in total.
using ProgressCallback = std::function<void(uint64_t extracted_bytes, uint64_t total_bytes)>;

// Opens the PKG and fills metadata. Returns error string on failure.
std::optional<std::string> ReadPkgMetadata(const std::filesystem::path& pkg_path, PkgMetadata& out);

// Extracts the PKG. If indices empty -> all files. Files are extracted on several threads;
// progress_cb is called on the calling thread about every 100 ms. Setting *cancel stops before
// the next file and returns an error.
std::optional<std::string> ExtractPkg(const std::filesystem::path& pkg_path,
                                      const std::filesystem::path& out_dir,
                                      const std::vector<int>& indices,
                                      const ProgressCallback& progress_cb,
                                      const std::atomic<bool>* cancel = nullptr);

// Helper to gather .pkg files in a directory (non-recursive or recursive as chosen).
std::vector<std::filesystem::path> ListPkgFiles(const std::filesystem::path& dir, bool recursive);
