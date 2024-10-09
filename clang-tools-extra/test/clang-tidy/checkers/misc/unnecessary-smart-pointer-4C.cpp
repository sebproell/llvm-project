// RUN: %check_clang_tidy %s misc-unnecessary-smart-pointer %t -- -config="{CheckOptions: {misc-unnecessary-smart-pointer.SmartPointerTypes: 'shared_ptr'}}" --

struct A{
  int a{};
};

namespace my{

// Use inheritance to better model std implementations.
template<typename T>
class shared_ptr_base{

public:
  shared_ptr_base() {}
  shared_ptr_base(T* ptr) {}
  ~shared_ptr_base() {delete ptr;}
  T& operator*() const { return *ptr; }
  T* get() { return ptr; }
  T* operator->() const { return ptr; }
private:
  T* ptr;
};

template<typename T>
class shared_ptr : public shared_ptr_base<T>{
public:
  shared_ptr() {}
  shared_ptr(T* ptr) : shared_ptr_base<T>(ptr) {}
};
}

void unnecessaryInParameter(my::shared_ptr<int> a){
// CHECK-MESSAGES: [[@LINE-1]]:29: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}void unnecessaryInParameter(int& a){{{$}}
  int b = *a;
}

void unnecessaryInParameter2(const my::shared_ptr<int> a){
// CHECK-MESSAGES: [[@LINE-1]]:30: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}void unnecessaryInParameter2(int& a){{{$}}
  int b = *a;
}

void unnecessaryInParameter3(const my::shared_ptr<int>& a){
// CHECK-MESSAGES: [[@LINE-1]]:30: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}void unnecessaryInParameter3(int& a){{{$}}
  int b = *a;
}

void unnecessaryInParameter4(const my::shared_ptr<const int>& a){
// CHECK-MESSAGES: [[@LINE-1]]:30: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}void unnecessaryInParameter4(const int& a){{{$}}
  int b = *a;
}

void unnecessaryInParameter5(my::shared_ptr<int>& a){
// CHECK-MESSAGES: [[@LINE-1]]:30: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}void unnecessaryInParameter5(int& a){{{$}}
  int b = *a;
}

auto lambdaNotMatched = [](my::shared_ptr<int> a){
  int b = *a;
};

void defaultArgSkipped(my::shared_ptr<int> a = nullptr){
  int b = *a;
}


void necessaryInParameter(my::shared_ptr<int> a){
  int b = *a;
  auto c = a;

  unnecessaryInParameter(a);
}

