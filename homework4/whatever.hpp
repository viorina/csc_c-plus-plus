#pragma once

#include <exception>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace utils {
    struct bad_whatever_cast : public std::exception {
        char const* what() const noexcept override {
            return "Conversion using whatever_cast failed.";
        }
    };

    struct whatever {
    public:
        whatever();
        whatever(std::initializer_list<std::pair<std::string const, whatever>> list);

        whatever(whatever const& other);
        whatever& operator=(whatever const& other);

        whatever(whatever&& other) noexcept;
        whatever& operator=(whatever&& other) noexcept;

        ~whatever();

        template<typename T> whatever(T const& obj);
        template<typename T> whatever& operator=(T const& obj);

        template<typename T> explicit whatever(T&& obj);
        template<typename T> whatever& operator=(T&& obj);

        void swap(whatever& other);

        std::type_info const& type_info() const;
        bool empty() const;
        void clear();

        template<typename T> friend T* whatever_cast(whatever* operand);
        template<typename T> friend T whatever_cast(whatever& operand);

        friend bool operator==(whatever const& lhs, whatever const& rhs);

    private:
        struct base_holder {
            virtual ~base_holder() = default;
            virtual base_holder* clone() const = 0;
            virtual const std::type_info& type_info() const = 0;
            virtual bool is_equal(base_holder* other) const = 0;
        };

        template<typename T>
        struct holder : base_holder {
            explicit holder(T const& value) : value_(value) {}
            explicit holder(T&& value) noexcept : value_(std::move(value)) {}

            base_holder* clone() const override {
                return new holder(value_);
            }

            std::type_info const& type_info() const override {
                return typeid(value_);
            }

            bool is_equal(base_holder* other) const override {
                return (type_info() == other->type_info())
                    && (value_ == static_cast<holder<T>*>(other)->value_);
            }

            T value_;
        };

        std::unique_ptr<base_holder> data_;
    };

    inline whatever::whatever() : data_(nullptr) {}

    inline whatever::whatever(std::initializer_list<std::pair<std::string const, whatever>> list) {
        whatever(std::unordered_map<std::string, whatever>(list));
    }

    inline whatever::whatever(whatever const& other) 
        : data_(other.data_ ? other.data_->clone() : nullptr) {}

    inline whatever& whatever::operator=(whatever const& other) {
        whatever(other).swap(*this);
        return *this;
    }

    inline whatever::whatever(whatever&& other) noexcept
        : data_(std::move(other.data_)) {}

    inline whatever& whatever::operator=(whatever&& other) noexcept {
        whatever(std::move(other)).swap(*this);
        return *this;
    }

    inline whatever::~whatever() {}

    template<typename T> 
    whatever::whatever(T const& obj) 
        : data_(new holder<typename std::decay<T>::type>(obj)) {}

    template<typename T> 
    whatever& whatever::operator=(T const& obj) {
        whatever(obj).swap(*this);
        return *this;
    }

    template<typename T> 
    whatever::whatever(T&& obj) 
        : data_(new holder<typename std::decay<T>::type>(std::forward<T>(obj))) {}

    template<typename T> 
    whatever& whatever::operator=(T&& obj) {
        whatever(std::forward<T>(obj)).swap(*this);
        return *this;
    }

    inline std::type_info const& whatever::type_info() const {
        return data_->type_info();
    }

    inline bool whatever::empty() const {
        return data_ == nullptr;
    }

    inline void whatever::clear() {
        whatever().swap(*this);
    }

    inline void whatever::swap(whatever& other) {
        std::swap(data_, other.data_);
    }

    inline void swap(whatever& lhs, whatever& rhs) {
        lhs.swap(rhs);
    }

    template<typename T>
    T* whatever_cast(whatever* operand) {
        if (operand && operand->data_ && operand->data_->type_info() == typeid(T)) {
            return &static_cast<whatever::holder<T>*>(operand->data_.get())->value_;
        } else {
            return nullptr;
        }
    }

    template<typename T>
    T const* whatever_cast(whatever const* operand) {
        return whatever_cast<T>(const_cast<whatever*>(operand));
    }

    template<typename T> 
    T whatever_cast(whatever& operand) {
        typedef typename std::decay<T>::type decayed;
        if (operand.data_->type_info() == typeid(decayed)) {
            return static_cast<whatever::holder<decayed>*>(operand.data_.get())->value_;
        } else {
            throw bad_whatever_cast();
        }
    }

    template<typename T> 
    T whatever_cast(whatever const& operand) {
        typedef typename std::decay<T>::type decayed;
        return whatever_cast<const decayed&>(const_cast<whatever&>(operand));
    }

    inline bool operator==(whatever const& lhs, whatever const& rhs) {
        return (lhs.empty() && rhs.empty()) || (lhs.data_.get()->is_equal(rhs.data_.get()));
    }
}
