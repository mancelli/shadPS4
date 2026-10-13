// SPDX-FileCopyrightText: Copyright 2025-2026 shadLauncher4 Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
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
                                      const ProgressCallback& progress_cb,
                                      const std::atomic<bool>* cancel) try {
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

    // Decryption and decompression are CPU bound: extract several files at once, like the
    // shadPS4 Qt launcher did with QtConcurrent::map.
    const uint64_t total_bytes = pkg.GetTotalFileBytes();
    std::atomic<size_t> next{0};
    std::atomic<bool> stop{false};
    std::mutex error_mutex;
    std::optional<std::string> error;
    const auto set_error = [&](std::string message) {
        std::scoped_lock lk{error_mutex};
        if (!error) {
            error = std::move(message);
        }
        stop = true;
    };
    const unsigned num_threads = std::clamp(std::thread::hardware_concurrency(), 1u, 8u);
    std::atomic<unsigned> running{num_threads};
    const auto worker = [&] {
        while (!stop && !(cancel && cancel->load())) {
            const size_t i = next++;
            if (i >= list.size()) {
                break;
            }
            const int idx = list[i];
            try {
                pkg.ExtractFiles(idx);
            } catch (const std::exception& e) {
                set_error(std::string("Failed extracting file index ") + std::to_string(idx) +
                          ": " + e.what());
            } catch (...) {
                set_error(std::string("Unknown error extracting file index ") +
                          std::to_string(idx));
            }
        }
        --running;
    };
    std::vector<std::thread> threads;
    for (unsigned t = 0; t < num_threads; ++t) {
        try {
            threads.emplace_back(worker);
        } catch (const std::exception& e) {
            set_error(std::string("Failed to start an extraction thread: ") + e.what());
            running -= num_threads - t;
            break;
        }
    }
    while (running > 0) {
        if (progress_cb) {
            progress_cb(pkg.GetExtractedBytes(), total_bytes);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    for (auto& thread : threads) {
        thread.join();
    }
    if (error) {
        return error;
    }
    if (cancel && cancel->load()) {
        return std::string("Installation cancelled");
    }
    if (progress_cb) {
        progress_cb(pkg.GetExtractedBytes(), total_bytes);
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
