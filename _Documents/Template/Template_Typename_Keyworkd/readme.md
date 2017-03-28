
# typename 키워드, template 키워드

* template 키워드는 템플릿 선언과 정의를 도입하는데 사용

```cpp
template <class _T>
class vector;
```

* typename 키워드는 템플릿 매개 변수를 선언할때 사용 \(class와 동일\) 

```cpp
template <typename _T>
class vector;
```

# 논점

* 템플릿의 컴파일은 두 단계로 이루어 진다.
    * 정의 지점\(point of definition\)  
    템플릿의 구문적인 명확함을 점검  
    인스터스화 전에 적격 여부를 판단
        ```cpp
        template <typename _Iter>
        void IterSwap(_Iter i1, _Iter i2)
        {
            _T temp = i1; // 오류 알려지지 않은 식별자 _T
            *i1 = *i2;
            *i2 = temp;
        }
        ```

    * 인스턴스화 지점\(point of instantiation\)  
    
## 문제 사례

* 형식의 모호성 (타입)

    > iter_trait\<_Iter\>::value_type* pi = &*i;

    1. ::value_type 이 하나의 형식일 경우 **유효한 선언**
    1. ::value_type 열거 값이라면 에러
    1. 하지만 아래와 같은 경우 형식이 아니라도 유효  
    **\(::value_type * pi\) = \&\*i**
        ```cpp
        struct num
        {
            template <class _U>
            num& operator=(const _U&);
            int& operator*() const;
        };

        num operator*(num, double);

        template <typename _T>
        struct iter_trait
        {
            static num value_type; //타입이 아니다.!
        };
        ```
    1. 정의 지점에서 iter_trait를 안다고 해도 특수화로 인해 구문 점검 불가능

    <br/>

    > **typename** iter_trait\<_Iter\>::value_type* pi = &*i;

    * 해결 방안  
    **typename** 키워드 사용, 의존적인 네임이 형식임을 나타낸다

<br/>

* 형식의 모호성 (템플릿 멤버)

    > template \<typename _T \>  
    > int f\(_T x\)  
    > \{  
    >   return **x.convert\<3\>\(pi\)**  
    > \}

    멤버 함수 템플릿 또는 자료 멤버 일수 있다.
    
    1. 멤버 함수 템플릿 일 경우  
    pi 를 인수로 convert\<3\>\(pi\)을 호출

    1. 자료 멤버일 경우  
    \(x.convert \< 3\) \> pi 로 계산
    
    <br/>

    > x.**template** convert\<3\>\(pi\)

    * 해결 방안  
    **template** 키워드 사용 \(멤버 템플릿\)

# 규칙

* typename 을 꼭 붙여야 하는 경우

1. 템플릿 안에서 형식을 뜻하는 한정된 이름(::이 붙여지는 경우)

* template 을 꼭 붙여야 하는 경우

1. \. -\> :: 한정을 통해 멤버 템플릿에 접근하는 의존적인 이름


