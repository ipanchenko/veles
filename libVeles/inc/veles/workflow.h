/*! @file workflow.h
 *  @brief VELES Workflow
 *  @author Ernesto Sanches <ernestosanches@gmail.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright 2013 Samsung R&D Institute Russia
 */

#ifndef INC_WORKFLOW_H_
#define INC_WORKFLOW_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <veles/logger.h>
#include <veles/unit.h>
#include <veles/make_unique.h>

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif

namespace veles {

/// Type that contains Unit properties
typedef std::unordered_map<std::string, std::shared_ptr<void>> PropertiesTable;

/// Properties sequence.
typedef std::vector<std::shared_ptr<void>> PropertiesSequence;

/** @brief VELES workflow */
class Workflow : protected DefaultLogger<Workflow, Logger::COLOR_ORANGE> {
 public:
  virtual ~Workflow() = default;
  /** @brief Appends a unit to the end of workflow
   *  @param unit VELES unit
   */
  void Add(const std::shared_ptr<Unit>& unit) {
    units_.push_back(unit);
  }
  /** @brief Clears the Workflow
   */
  void Clear() {
    units_.clear();
  }
  /** @brief Number of units
   */
  size_t Size() const noexcept {
    return units_.size();
  }
  /* @brief Number of workflow inputs
   */
  size_t InputCount() const noexcept {
    return Size() ? units_.front()->InputCount() : 0;
  }
  /* @brief Number of workflow outputs
   */
  size_t OutputCount() const noexcept {
    return Size() ? units_.back()->OutputCount() : 0;
  }
  /** @brief Sets a parameter of the workflow
   *  @param name Name of the parameter
   *  @param value Pointer to the parameter value
   */
  void SetProperty(const std::string& name,
                    std::shared_ptr<void> value);
  /** @brief Returns a parameter of the workflow
   *  @param name Name of the parameter
   */
  std::shared_ptr<void> GetProperty(const std::string& name);
  /** @brief Returns a unit from workflow
   *  @param index Unit position in workflow
   */
  std::shared_ptr<Unit> Get(size_t index) const;
  /** @brief Executes the workflow
   *  @param begin Iterator to the first element of initial data
   *  @param end Iterator to the end of initial data
   *  @param out Output iterator for the result
   */
  template<class InputIterator, class OutputIterator>
  void Execute(InputIterator begin, InputIterator end,
               OutputIterator out) const {
    size_t max_size = MaxUnitSize();
    auto input = std::uniquify(mallocf(max_size), std::free);
    auto output = std::uniquify(mallocf(max_size), std::free);
    std::copy(begin, end, input.get());

    float* curr_in = input.get();
    float* curr_out = output.get();
    if (!units_.empty()) {
      for (const auto& unit : units_) {
        unit->Execute(curr_in, curr_out);
        std::swap(curr_in, curr_out);
      }
      std::copy(curr_in, curr_in + units_.back()->OutputCount(), out);
    }
  }

  static float* mallocf(size_t length);

 private:
  friend class WorkflowLoader;
  /** @brief Get maximum input and output size of containing units
   *  @return Maximum size
   */
  size_t MaxUnitSize() const noexcept;
  void SetProperties(const PropertiesTable& table);

  std::vector<std::shared_ptr<Unit>> units_;
  PropertiesTable props_;
};

}  // namespace veles

#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif

#endif  // INC_WORKFLOW_H_
