#pragma once

namespace seop::input
{
#define KEY_LIST(v) \
    v(LButton, 0x01) \
    v(RButton, 0x02) \
    v(MButton, 0x04) \
    v(Space, 0x20) \
    v(End, 0x23) \
    v(Left, 0x25) \
    v(Up, 0x26) \
    v(Right, 0x27) \
    v(Down, 0x28) \
    v(Delete, 0x2E) \
    v(A, 0x41) \
    v(B, 0x42) \
    v(C, 0x43) \
    v(D, 0x44) \
    v(E, 0x45) \
    v(F, 0x46) \
    v(G, 0x47) \
    v(H, 0x48) \
    v(I, 0x49) \
    v(J, 0x4A) \
    v(K, 0x4B) \
    v(L, 0x4C) \
    v(M, 0x4D) \
    v(N, 0x4E) \
    v(O, 0x4F) \
    v(P, 0x50) \
    v(Q, 0x51) \
    v(R, 0x52) \
    v(S, 0x53) \
    v(T, 0x54) \
    v(U, 0x55) \
    v(V, 0x56) \
    v(W, 0x57) \
    v(X, 0x58) \
    v(Y, 0x59) \
    v(Z, 0x5A) \
    v(LeftShift, 0xA0) \
    v(RightShift, 0xA1) \
    v(LeftControl, 0xA2) \
    v(RightControl, 0xA3) \
    v(LeftAlt, 0xA4) \
    v(RightAlt, 0xA5)

enum class Key_state
{
    Down,
    Pressed,
    Up,
    Released,
};
enum class Key_code : int
{
#define ENUM_GEN(name, code) name = code,
    KEY_LIST(ENUM_GEN)
#undef ENUM_GEN
};

inline const char *get_key_string(Key_code key)
{
    switch (key) {
#define NAME_GEN(name, code)                                                                                           \
    case Key_code::name:                                                                                               \
        return #name;

        KEY_LIST(NAME_GEN)

#undef NAME_GEN
    default:
        return "Unknown";
    }
}
} // namespace seop::input