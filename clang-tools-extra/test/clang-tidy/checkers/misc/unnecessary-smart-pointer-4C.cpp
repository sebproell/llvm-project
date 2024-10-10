// RUN: %check_clang_tidy %s misc-unnecessary-smart-pointer %t -- -config="{CheckOptions: {misc-unnecessary-smart-pointer.SmartPointerTypes: 'shared_ptr'}}" --

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

template<typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args){
return shared_ptr<T>();
}
}

void unnecessaryInParameter(my::shared_ptr<int> a){
// CHECK-MESSAGES: [[@LINE-1]]:29: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}void unnecessaryInParameter(int& a){{{$}}
  int b = *a;
// CHECK-FIXES: {{^}}  int b = a;{{$}}
}

void unnecessaryInParameter2(const my::shared_ptr<int> a){
// CHECK-MESSAGES: [[@LINE-1]]:30: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}void unnecessaryInParameter2(int& a){{{$}}
  int b = *a;
// CHECK-FIXES: {{^}}  int b = a;{{$}}
}

void unnecessaryInParameter3(const my::shared_ptr<int>& a){
// CHECK-MESSAGES: [[@LINE-1]]:30: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}void unnecessaryInParameter3(int& a){{{$}}
  int b = *a;
// CHECK-FIXES: {{^}}  int b = a;{{$}}
}

void unnecessaryInParameter4(const my::shared_ptr<const int>& a){
// CHECK-MESSAGES: [[@LINE-1]]:30: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}void unnecessaryInParameter4(const int& a){{{$}}
  int b = *a;
// CHECK-FIXES: {{^}}  int b = a;{{$}}
}

void unnecessaryInParameter5(my::shared_ptr<int>& a){
// CHECK-MESSAGES: [[@LINE-1]]:30: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}void unnecessaryInParameter5(int& a){{{$}}
  int b = *a;
// CHECK-FIXES: {{^}}  int b = a;{{$}}
}


void unnecessaryDecl(){
  my::shared_ptr<int> a =
      my::make_shared<int>();
// CHECK-MESSAGES: [[@LINE-2]]:3: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}  int a;{{$}}
  int b = *a;
// CHECK-FIXES: {{^}}  int b = a;{{$}}
}

void unnecessaryDecl2(){
  my::shared_ptr<int> abc = my::make_shared<int>(5);
// CHECK-MESSAGES: [[@LINE-1]]:3: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}  int abc(5);{{$}}
}

namespace A::B::C {
  struct S{};
}

void unnecessaryDecl3(){
  auto abc = my::make_shared<A::B::C::S>();
// CHECK-MESSAGES: [[@LINE-1]]:3: warning: this smart pointer is unnecessary
// CHECK-FIXES: {{^}}  A::B::C::S abc;{{$}}
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

