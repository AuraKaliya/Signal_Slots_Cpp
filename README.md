# Signal_Slots_Cpp
A simple signal-slot mechanism is implemented（ONLY MinGW！）

2024.4.28

Optimized with C17's std::tuple and std::apply, MSVC and MinGW are now available.

## Example

```c++
//define Class
class A
{
public:
    A() :m_value(10)
        {};

    static void slotS() {
        //std::cout << "slotS ."<<std::endl;
        qDebug()<<"slotS .";
    }
    static void slotS2(int value ) {
        //std::cout << " slotS2  1ARG   value ="<< value << std::endl;
        qDebug()<<" slotS2  1ARG   value ="<< value ;
    }

    void slotA() {
       // std::cout << "SlotA by" << this << " Value:" << m_value << std::endl;

        qDebug()<<"SlotA by" << this << " Value:" << m_value ;
    };
    void slotB(int value) {
        //std::cout << "SlotB  1 ARG by" << this << " Value:" << value << std::endl;

        qDebug()<<"SlotB  1 ARG by" << this << " Value:" << value ;
    };

    void slotC(int value,bool flag) {
        //std::cout << "SlotC  2 ARG  by" << this << " Value:" << value <<" Flag:"<< flag << std::endl;
        qDebug()<< "SlotC  2 ARG  by" << this << " Value:" << value <<" Flag:"<< flag ;

    };

    int value() { return m_value; }
    void add() { m_value++; };
private:
    int m_value;
};
```

```c++
 		A a;
        A b;
        qDebug()<<" a:" << &a ;
        qDebug()<<" b:" << &b ;
        qDebug()<<"1===================";

        REGISTER_SIGNAL(&a, &b, "TestSignal", &A::slotA);
        REGISTER_SIGNAL(&a, &a, "TestSignal", &A::slotA);

        REGISTER_SIGNAL(&a, &a, "TestSignal", &A::slotB,int);

        REGISTER_SIGNAL(&a, &b, "TestSignal0", &A::slotC, int,bool);
        REGISTER_SIGNAL_STATIC(&a, "TestSignal2", &A::slotS2, int);
        REGISTER_SIGNAL_STATIC(&a, "TestSignal2",&A::slotS);

        qDebug()<<"2===================";
        // multi binds
        EMIT_SIGNAL(&a, "TestSignal");
        qDebug()<<"3===================";
        // args
        EMIT_SIGNAL(&a, "TestSignal", b.value());;
        EMIT_SIGNAL(&a, "TestSignal0", b.value(), true);
        qDebug()<<"4===================";
        // static
        EMIT_SIGNAL(&a, "TestSignal2", b.value());
        EMIT_SIGNAL(&a, "TestSignal2");
        qDebug()<<"5===================";
        // no bind
        EMIT_SIGNAL(&b, "TestSignal0", b.value(), true);
        qDebug()<<"6===================";
```

