#pragma once
template<typename T>
class DxcPtr {
    T* ptr;
    bool contained;
    void dispose() {
        if (ptr && contained) {
            ptr->Release();
        }
    }

public:
    operator bool() const noexcept { return contained; }
    void clear() noexcept {
        dispose();
        ptr = nullptr;
        contained = false;
    }
    DxcPtr(T* ptr, bool contained) noexcept
        : ptr{ptr}, contained{contained} {}
    DxcPtr() noexcept : DxcPtr{nullptr, false} {}
    DxcPtr(DxcPtr const&) = delete;
    DxcPtr(DxcPtr&& rhs) noexcept {
        ptr = rhs.ptr;
        contained = rhs.contained;
        rhs.ptr = nullptr;
        rhs.contained = false;
    }
    DxcPtr& operator=(DxcPtr const&) noexcept = delete;
    DxcPtr& operator=(DxcPtr&& rhs) noexcept {
        dispose();
        ptr = rhs.ptr;
        contained = rhs.contained;
        rhs.ptr = nullptr;
        rhs.contained = false;
        return *this;
    }
    T** GetAddressOf() noexcept {
        dispose();
        ptr = nullptr;
        contained = true;
        return &ptr;
    }
    T* Get() const noexcept {
        return ptr;
    }
    T* operator->() const noexcept {
        return ptr;
    }
    T& operator*() const noexcept {
        return *ptr;
    }
    ~DxcPtr() {
        dispose();
    }
    operator T*() const noexcept {
        return ptr;
    }
};