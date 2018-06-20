/**
 * Based on StackOverflow answer at:
 * https://stackoverflow.com/a/1120224
 * By: https://stackoverflow.com/users/14065/martin-york
 */
#ifndef TDHH_CSVITERATOR_HPP
#define TDHH_CSVITERATOR_HPP

#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

namespace TDHH {
    class CSVRow {
    private:
        std::vector<std::string> m_data;

    public:
        std::string const &operator[](std::size_t index) const {
            return m_data[index];
        }

        std::size_t size() const {
            return m_data.size();
        }

        void readNextRow(std::istream &str) {
            std::string line;
            std::getline(str, line);

            std::stringstream lineStream(line);
            std::string cell;

            m_data.clear();
            while (std::getline(lineStream, cell, ',')) {
                m_data.push_back(cell);
            }
            // This checks for a trailing comma with no data after it.
            if (!lineStream && cell.empty()) {
                // If there was a trailing comma then add an empty element.
                m_data.emplace_back("");
            }
        }
    };

    inline std::istream &operator>>(std::istream &str, CSVRow &data) {
        data.readNextRow(str);
        return str;
    }

    class CSVIterator {
    private:
        std::istream *m_str;
        CSVRow m_row;

    public:
        explicit CSVIterator(std::istream &str) : m_str(str.good() ? &str : nullptr) { ++(*this); }
        CSVIterator() : m_str(nullptr) {}

        // Pre Increment
        CSVIterator &operator++() {
            if (m_str) { if (!((*m_str) >> m_row)) { m_str = nullptr; }}
            return *this;
        }

        // Post increment
        const CSVIterator operator++(int) {
            CSVIterator tmp(*this);
            ++(*this);
            return tmp;
        }

        CSVRow const &operator*() const { return m_row; }

        CSVRow const *operator->() const { return &m_row; }

        bool operator==(CSVIterator const &rhs) {
            return ((this == &rhs) || ((this->m_str == nullptr) && (rhs.m_str == nullptr)));
        }

        bool operator!=(CSVIterator const &rhs) { return !((*this) == rhs); }
    };
}

#endif //TDHH_CSVITERATOR_HPP
