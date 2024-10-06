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
  T& operator*() { return *ptr; }
  T* get() { return ptr; }
  T* operator->() { return ptr; }
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
  int b = *a;
}

void necessary(){
  // This shared_ptr cannot be determined to be unnecessary since we pass it to
  // a function and call get().
  my::shared_ptr<int> a(new int(1));
  int b = *a;
  auto* c = a.get();
  unnecessaryInParameter(a);
}

void unnecessaryInFunction(){
  int something_else = 0;
  my::shared_ptr<A> a = my::shared_ptr<A>(new A);
// CHECK-MESSAGES: [[@LINE-1]]:3: warning: this smart pointer is unnecessary
  A b = *a;
  a->a = 1;
}

void unnecessaryInBlock(){
{
  my::shared_ptr<A> a = my::shared_ptr<A>(new A);
// CHECK-MESSAGES: [[@LINE-1]]:3: warning: this smart pointer is unnecessary
  A b = *a;
  a->a = 1;
}
}
