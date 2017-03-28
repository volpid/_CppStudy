
## 메타프로그램 in C++

메타프로그램을 단어 그대로 해석한다면 "프로그램에 대한 프로그램", 즉 코드(프로그램)을 조작하는 프로그램이다.
<br/>
C++ 컴파일러는 C++코드를 조작해서 어셈블리어 또는 기계어를 만들어 낸다.
<br/>
이런 의미에서 C++ 컴파일러도 바로 메타프로그램으로 볼 수 있다.
<br/>
<br/>
가장 대표되는 아래와 같은 C++메타프로그램은 컴파일 시점 계산 **[수치계산]** 이다. 

```cpp
template <unsigned int _N>
struct BinaryTemplate
{
public:
    static const unsigned int value = BinaryTemplate<_N / 10>::value * 2 + _N % 10;
};

template <>
struct BinaryTemplate<0>
{
    static const unsigned int value = 0;
};
```

BinaryTemplate<_N>::value 에 접근하게 될때 N = 0이 될 때가지 작은 N에 대해 인스턴스화 하게 된다.
<br/>
0에 대한 **템플릿 특수화(tempalte speicializatoin)는 재귀의 종료 조건**으로 사용된다.
<br/>
프로그램이란 것도 하나의 함수라는 사실에 비쳐보면 본질적으로 이 작은 메타프로그램은 컴파일러가 컴파일 도중에 **해석(Interpret)** 하나의 프로그램이 된다.
<br/>
<br/>
C++ 언어는 컴파일 시점에서의 표현식과 실행시점 계산을 구분하므로, 메타프로그램과 그에 해당하는 실행시점 코드는 모습이 다를 수 밖에 없다.
<br/>
위 binary의 실행 시점의 버전은 아래와 같다.
<br/>
재귀 또는 for 문을 사용한 구현이 가능하다.

```cpp
unsigned int BinaryRecursiveFunc(unsigned int n)
{
    return (n == 0) ? (0) : (n % 10 + 2 * BinaryRecursiveFunc(n / 10));
}

unsigned int BinaryForFunc(unsigned int n)
{
    unsigned int result = 0;
    for (unsigned int bit = 0x1; n; n /= 10, bit <<= 1)
    {
        if (n % 10)
        {
            result += bit;
        }
    }
    return result;
}
```

constexpr 등장이후 재귀를 사용한 함수는 컴파일타임 수치계산 함수로 사용 가능하다.

```
constexpr unsigned int BinaryRecursiveFunc(unsigned int n)
{
    return (n == 0) ? (0) : (n % 10 + 2 * BinaryRecursiveFunc(n / 10));
}
```

c++ 템플릿 메타프로그래밍에서, 컴파일 시점 수치계산 보다 중요한 것은 **형식\(type\)** 들을 계산하는 능력이다.

```
expr = 
    (term[expr.val = _1) >> '+' >> expr[expr.val += _1])
    | (term[expr.val = _1] >> '-' >> expr[expr.val -= _1])
    | term[expr.val = _1]
    ;
    
term = 
    (factor[term.val = _1] >> "*" >> term[term.val *= _1])
    | (factor[term.val = _1] >> "/" >> term[term.val /= _1])
    | factor[term.val]
    ;
    
factor =
    integer[factor.val = _1]
    | ('(' >> expr[factor.val = _1] >> ')')
    ;
```

이 코드에서, 각각의 배정문은 그 우변에 있는 문법 규칙을 해석, 평가하는 함수 객체를 좌변의 변수에 저장한다.
<br/>
저장된 각 함수 객체의 행동 방식은 전적으로 함수 객체를 생성할 때 쓰인 표현식의 **형식**의해 결정된다.
<br/>
각 표현식의 형식은 표현식에 쓰인 여러 연산자들에 연관된 **메타프로그램이 계산한다**
<br/>

***

## 메타프로그래밍이 필요한 이유

### **메타프로그래밍의 이점은 무엇일까?**

1. 실행 시점 계산
> 메타프로그래밍의 경우 실제 프로그램이 시작되기 전에 최대한 많은 일을 해 둠으로써 **더 빠른 프로그래**을 얻게 다는데 있다. 
> <br/>
> unsigned int BinaryRecursiveFunc(unsigned int n) 함수의 경우 실행시점에 주어진 입렵에 따른 재귀 함수를 실행하게 된다.
> <br/>
> 하지만, BinaryTemplate<> 또는 constexpr 버전의 경우 컴파일시점에 계산하여 실제 바이너리 코드에는 상수값으로 치환된다.
> <br/>
> 물론 이로인해 컴파일 타임이 증가한다.

2. 사용자의 해석
> 메타프로그램을 작성해서 단 한번 정도만 사용하는 경우라면, 그냥 프로그래머가 직접 처리하는 것이 더 간편할 것이다.
> <br/>
> 그러나 빈번하게 일어나는 작업의 경우 메타 프로그램을 만들어서 계속 사용하는 편이 더 편리하다.
> <br/>
> 그리고 사용횟수와 무관하게 메타프로그래밍은 **좀 더 표현력 있는 코드**를 만들수 있게한다.
> <br/>
> binary의 경우, 이진수 비트 하나가 의미 있는경우 42나 0x2a보다는 binary<101010>::value가 이해하기 쉽다.
> <br/>

3. C++에서 메타프로그래밍
> 새로운 문법을 배우지 않고, 코드의 흐름을 깨지 않고도 영역언어에 직접 진입할 수 있다.
> <br/>
> 다른 코드(특히 다른 메타프로그램)와 메타코드의 연동이 훨씬 더 매끄럽다.
> <br/>
> 추가적인 빌드 단계가 필요하지 않다.