#ifndef LIBZ11_LIST_H
#define LIBZ11_LIST_H

namespace z11 {
template <class T>
class List
{
 public:
  List();
  List(T* data);

  void Insert(List<T>* elem);
  void Remove();
  bool Empty();
  bool Head();
  List<T>* prev();
  List<T>* next();
  T* data();

 private:
  List<T>* prev_;
  List<T>* next_;
  T* data_;
};

template <class T>
inline List<T>* List<T>::prev()
{
  return prev_;
}

template <class T>
inline List<T>* List<T>::next()
{
  return next_;
}

template <class T>
inline T* List<T>::data()
{
  return data_;
}

template <class T>
List<T>::List()
{
  this->data_ = nullptr;
  this->next_ = this;
  this->prev_ = this;
}

template <class T>
List<T>::List(T* data)
{
  this->data_ = data;
  this->next_ = nullptr;
  this->prev_ = nullptr;
}

template <class T>
void List<T>::Insert(List<T>* elem)
{
  elem->prev_ = this;
  elem->next_ = this->next_;
  this->next_ = elem;
  elem->next_->prev_ = elem;
}

template <class T>
void List<T>::Remove()
{
  this->prev_->next_ = this->next_;
  this->next_->prev_ = this->prev_;
  this->prev_ = nullptr;
  this->next_ = nullptr;
}

template <class T>
bool List<T>::Empty()
{
  return this->next_ == this;
}

template <class T>
bool List<T>::Head()
{
  return this->data_ == nullptr;
}

}  // namespace z11

#endif  // LIBZ11_LIST_H
