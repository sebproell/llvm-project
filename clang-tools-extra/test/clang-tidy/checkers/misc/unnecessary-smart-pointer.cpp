// RUN: %check_clang_tidy %s misc-unnecessary-smart-pointer %t -- -config="{CheckOptions: {misc-unnecessary-smart-pointer.SmartPointerTypes: 'shared_ptr'}}" --

template<typename T>
class shared_ptr {
public:
  shared_ptr() {}
  shared_ptr(T* ptr) {}
  ~shared_ptr() {delete ptr;}
  T operator*() { return T(); }
private:
  T* ptr;
};

void g(shared_ptr<int> a){
  int b = *a;
}

void f(){
  shared_ptr<int> a(new int(1));
  int b = *a;
  g(a);
}
