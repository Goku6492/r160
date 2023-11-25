#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <gmpxx.h>

std::string generate_key(const std::string& prefix) {
    mpz_class random_number = mpz_class(gmp_randinit_default());
    mpz_urandomb(random_number.get_mpz_t(), gmp_randstate_t(nullptr), 256);

    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(64) << random_number;
    return prefix + stream.str();
}

std::string sha256_and_ripemd160(const std::string& public_key) {
    SHA256_CTX sha256;
    RIPEMD160_CTX ripemd160;
    unsigned char sha256_hash[SHA256_DIGEST_LENGTH];
    unsigned char ripemd160_hash[RIPEMD160_DIGEST_LENGTH];

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, public_key.c_str(), public_key.size());
    SHA256_Final(sha256_hash, &sha256);

    RIPEMD160_Init(&ripemd160);
    RIPEMD160_Update(&ripemd160, sha256_hash, SHA256_DIGEST_LENGTH);
    RIPEMD160_Final(ripemd160_hash, &ripemd160);

    std::stringstream stream;
    stream << std::hex << std::setfill('0');
    for (int i = 0; i < RIPEMD160_DIGEST_LENGTH; ++i) {
        stream << std::setw(2) << static_cast<unsigned>(ripemd160_hash[i]);
    }

    return stream.str();
}

void check_and_save(const std::string& public_key, const std::string& rmd, const std::vector<std::string>& target_rmds, std::ofstream& outfile) {
    auto it = std::find(target_rmds.begin(), target_rmds.end(), rmd);
    if (it != target_rmds.end()) {
        std::cout << "Public Key: " << public_key << ", RMD160: " << rmd << std::endl;
        outfile << "Public Key: " << public_key << ", RMD160: " << rmd << std::endl;
    }
}

void worker(const std::string& prefix, const std::vector<std::string>& target_rmds, std::ofstream& outfile) {
    while (true) {
        std::string key = generate_key(prefix);
        std::string rmd = sha256_and_ripemd160(key);
        check_and_save(key, rmd, target_rmds, outfile);

        auto it = std::find_if(target_rmds.begin(), target_rmds.end(), [&rmd](const std::string& target) {
            return target == rmd;
        });

        if (it != target_rmds.end()) {
            target_rmds.erase(it);
        }

        if (target_rmds.empty()) {
            break;
        }
    }
}

int main() {
    std::vector<std::string> target_rmds = {
        "20d45a6a762535700ce9e0b216e31994335db8a5",
        "739437bb3dd6d1983e66629c5f08c70e52769371",
        // Add the rest of your RMD160 values here
    };

    std::ofstream outfile("foundrmd.txt");
    std::vector<std::string> prefixes = {"02", "03"};

    #pragma omp parallel for
    for (int i = 0; i < prefixes.size(); ++i) {
        worker(prefixes[i], target_rmds, outfile);
    }

    outfile.close();

    return 0;
}
