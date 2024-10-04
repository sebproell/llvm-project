// RUN: %check_clang_tidy %s misc-unnecessary-smart-pointer %t -- -config="{CheckOptions: {misc-unnecessary-smart-pointer.SmartPointerTypes: 'shared_ptr'}}" --

template<typename T>
class shared_ptr {
public:
  shared_ptr() {}
  shared_ptr(T* ptr) {}
  ~shared_ptr() {delete ptr;}
  T operator*() { return T(); }
  T* get() { return ptr; }
private:
  T* ptr;
};

void fn(shared_ptr<int> a){
// CHECK-MESSAGES: [[@LINE-1]]:9: warning: this smart pointer is unnecessary
  int b = *a;
}

void f(){
  // This shared_ptr cannot be determined to be unnecessary since we pass it to a function.
  shared_ptr<int> a(new int(1));
  int b = *a;
  auto* c = a.get();
  fn(a);
}

void g(){
  shared_ptr<int> a(new int(1));
// CHECK-MESSAGES: [[@LINE-1]]:3: warning: this smart pointer is unnecessary
  int b = *a;
  auto* c = a.get();
}
