#ifndef ENCRYPTION_HPP
#define ENCRYPTION_HPP

#include <gmpxx.h> // gmp.h for C
#include <QByteArray>

// Singleton class
class Encryption
{
private:
    // Can be changed, the higher the number,
    // the better the encryption
    static constexpr int BIT_ENCRYPTION {2048};

    // Number System
    static constexpr int BASE {10};

    // Module from the number 'n', which has 2*BIT_ENCRYPTION bits.
    // Get the number of bytes in the block.
    // The recommended padding is 42 bytes for 512 bytes or 11 for 256
    static constexpr int DECODE_BLOCK_SIZE {2 * BIT_ENCRYPTION / 8};
    static constexpr int ENCODE_BLOCK_SIZE {DECODE_BLOCK_SIZE - 42};

    // n = p * q, where p and q are prime numbers
    mpz_class n;

    // private key
    mpz_class d;

    // public key
    mpz_class e;

    static mpz_class createPrime();
    QByteArray encode_decode_block_bytes(const QByteArray& bytes, const QByteArray& cipher_key, bool encode) const;

    Encryption(const Encryption&)               = delete;
    Encryption(Encryption&&)                    = delete;
    Encryption& operator= (const Encryption&)   = delete;
    Encryption& operator= (Encryption&&)        = delete;
    Encryption();

public:
    static Encryption* get_instance() {
        static Encryption rsa = Encryption();
        return &rsa;
    }

    // UTF-8 format
    QByteArray get_n() const;

    QByteArray encode(const QByteArray& bytes, const QByteArray& key) const;
    QByteArray decode(const QByteArray& bytes) const;

};

#endif // ENCRYPTION_HPP
