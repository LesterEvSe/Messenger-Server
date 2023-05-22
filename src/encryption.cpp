#include "encryption.hpp"

#include <random>
#include <memory> // for smart pointer
#include <string>

Encryption::Encryption()
{
    // 1. Generate 2 prime numbers
    mpz_class p = createPrime();
    mpz_class q = createPrime();

    // 2. Calculate 'n'
    n = p * q;

    // 3. Calculate 'fi(n)'
    mpz_class fi_n = (p - mpz_class(1)) * (q - mpz_class(1));

    // 4. Selecting the recommended value of 'e'
    e = 65'537;

    // 5. Calculate private key;
    // mpz_class is a C++ class wrapper, for the C structure mpz_t.
    // get_mpz_t() is needed to get a raw pointer to mpz_t

    // d * e % fi(n) = 1 therefore
    // d = (e^-1) % fi(n)
    // Modular Multiplicative Inverse here
    mpz_invert(d.get_mpz_t(), e.get_mpz_t(), fi_n.get_mpz_t());
}

Encryption* Encryption::get_instance() {
    static Encryption rsa = Encryption();
    return &rsa;
}

QByteArray Encryption::get_n() const
{
    std::string str = n.get_str();

    // c_str return const char*
    return QByteArray(str.c_str(), str.size());
}

mpz_class Encryption::createPrime()
{
    mpz_class prime;
    gmp_randstate_t state;

    // Initializing a random number generator
    gmp_randinit_default(state);

    // For more accuracy
    std::random_device rd;
    std::random_device::result_type seed = rd();

    // Set a random generator state
    // seed.count() - get the number of ticks
    gmp_randseed_ui(state, seed);

    // Generating a 2048-bit prime number
    do {
        prime = mpz_class(1);

        // Generating a random number
        mpz_urandomb(prime.get_mpz_t(), state, BIT_ENCRYPTION);

        // Finding the nearest simple to the generated one
        mpz_nextprime(prime.get_mpz_t(), prime.get_mpz_t());

        // Check that the number has exactly BIT_ENCRYPTION bits
    } while (mpz_sizeinbase(prime.get_mpz_t(), 2) != BIT_ENCRYPTION);

    // To check the simplicity of a number by the Miller-Rabin test
    // 25 - number of check iterations. A reasonable limit is 15-50

    // 0 - the number is definitely not prime
    // 1 - the number is prime with a probability of 50% or close to this
    // 2 - Prime with a high probability
    if (!mpz_probab_prime_p(prime.get_mpz_t(), 25))
        return createPrime();

    // free resources
    gmp_randclear(state);

    return prime;
}

// encrypt, if true, otherwise decode
QByteArray Encryption::encode_decode_block_bytes(const QByteArray& bytes, const QByteArray& cipher_key, bool encode) const
{
    const unsigned char* data = reinterpret_cast<const unsigned char*>(bytes.data());
    size_t num_bytes = static_cast<size_t>(bytes.size());
    mpz_class num, res;

    // 1 - Little-Endian order (The least significant byte at the beginning)
    // first 0  (endian) - Byte order for each item on a given platform
    // second 0 (nails)  - Several low of each bytes that are not imported.
    // All significant, so we import without skips
    mpz_import(num.get_mpz_t(), num_bytes, 1, sizeof(data[0]), 0, 0, data);

    // res = num ^ e % n
    if (encode) {
        mpz_class mod;
        mod.set_str(cipher_key.constData(), BASE);

        mpz_powm(res.get_mpz_t(), num.get_mpz_t(), e.get_mpz_t(), mod.get_mpz_t());
    }
    else
        mpz_powm(res.get_mpz_t(), num.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());

    // 256 base system is (1 << 8)
    size_t res_size = mpz_sizeinbase(res.get_mpz_t(), 256);
    std::unique_ptr<unsigned char[]> res_data = std::make_unique<unsigned char[]>(res_size);

    size_t exported_size;
    mpz_export(res_data.get(), &exported_size, 1, sizeof(res_data.get()[0]), 0, 0, res.get_mpz_t());

    QByteArray answer(reinterpret_cast<char*>(res_data.get()), static_cast<int>(exported_size));
    return answer;
}

QByteArray Encryption::encode(const QByteArray& bytes, const QByteArray& key) const
{
    QByteArray answer;
    for (int i = 0; i < bytes.size(); i += ENCODE_BLOCK_SIZE) {
        QByteArray block(bytes.mid(i, ENCODE_BLOCK_SIZE));
        answer += encode_decode_block_bytes(block, key, true);
    }
    return answer;
}

QByteArray Encryption::decode(const QByteArray& bytes) const
{
    // Use private key for decoding, so we do not need the QByteArray parameter
    // Pass in it an empty.
    QByteArray plug;

    QByteArray answer;
    for (int i = 0; i < bytes.size(); i += DECODE_BLOCK_SIZE) {
        QByteArray block(bytes.mid(i, DECODE_BLOCK_SIZE));
        answer += encode_decode_block_bytes(block, plug, false);
    }
    return answer;
}
