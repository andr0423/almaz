#include <iostream>
#include <fstream>
#include <bitset>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

bool eof = false;
std::ifstream datafile;
size_t batch_size;

std::mutex * mm[2];
std::condition_variable * ccv[2];
std::queue<long long> * qq[2];


void action_read(const uint queue_max_size, const uint queue_min_size){
    long long next_value;
    
    bool sskp[2] { false, false };
    size_t rr = 0;
    uint qs = 0;
    while( ! eof || qs > 0 ) {
        rr ^= 1;

        {
            std::lock_guard<std::mutex> lk(*mm[rr]);
            qs = (uint)qq[rr]->size();
        }
        
        if ( qs > queue_max_size ) {
            sskp[rr] = true;
        }
        if ( sskp[rr] ) {
            if ( qs < queue_min_size ) {
                sskp[rr] = false;
            }
            ccv[rr]->notify_all();
            continue;
        }

        {
            size_t repeat = 10;
            std::lock_guard<std::mutex> lk(*mm[rr]);
            while ( ! eof && repeat > 0 ){
                repeat--;
                if ( datafile >> next_value ) {
                    qq[rr]->push(next_value);
                } else {
                    eof = true;
                }
            }
            qs = (uint)qq[rr]->size();
        }

        ccv[rr]->notify_all();
    }
}

void action_sum( const size_t n , long long & sm , long long & xr ) {
    long long sum_th = 0;
    long long xor_th = 0;
    while( true ) {
        
        std::unique_lock<std::mutex> lk(*mm[n]);

        while( qq[n]->empty() && ! eof ) {
            ccv[n]->wait(lk);
        }
        if ( eof && qq[n]->empty() ) {
            lk.unlock();
            break;
        }

        size_t repeat = 10;
        while ( repeat > 0 && ! qq[n]->empty() ) {
            repeat--;

            long long next_value = qq[n]->front();
            qq[n]->pop();

            sum_th += next_value;
            xor_th ^= next_value;
        }

        lk.unlock();

    }
    sm = sum_th;
    xr = xor_th;
}

int main(int argc, char *argv[]){
    batch_size = 10;
    uint queue_max_size = 255;
    uint queue_min_size = 127;

    long long res_summary;
    long long res_subtract;
    long long res_xor_all;

    long long first_value;
    long long res_sum_1 = 0, res_sum_2 = 0;
    long long res_xor_1 = 0, res_xor_2 = 0;

    if (argc != 2) { 
        std::cerr << "Expected file name as argument.\n" 
                  <<  "Usage: " << argv[0] << " <filename>\n";
        return 1; 
    }
    datafile.open(argv[1]);
    if ( ! datafile.is_open()) {
        std::cerr << "File '" << argv[1] << "' not exist or inaccessible...\n";
        return 2;
    }

    for (size_t i = 0; i<2; i++) {
        mm[i]  = new std::mutex();
        ccv[i] = new std::condition_variable();
        qq[i]  = new std::queue<long long>();
    }

    datafile >> first_value;

    std::thread thr_reader( action_read, queue_max_size, queue_min_size );
    std::thread thr_sum   ( action_sum, 0, std::ref(res_sum_1), std::ref(res_xor_1) );
    std::thread thr_sum2  ( action_sum, 1, std::ref(res_sum_2), std::ref(res_xor_2) );

    thr_reader.join();
    thr_sum.join();
    thr_sum2.join();

    long long thread_sum = res_sum_1 + res_sum_2;

    res_summary  = first_value + thread_sum;
    res_subtract = first_value - thread_sum;
    res_xor_all  = first_value ^ res_xor_1 ^ res_xor_2;

    std::bitset<64> res_xor_bin2(res_xor_all);

    std::cout << "Summary:     " << res_summary  << std::endl
              << "Subtraction: " << res_subtract << std::endl
              << "XOR:         " << res_xor_all  << ", '"
                                 << res_xor_bin2.to_string() << "'"
              << std::endl;

    for (size_t i = 0; i<2; i++) {
        delete(mm[i]);
        delete(ccv[i]);
        delete(qq[i]);
    }
    datafile.close();
    return 0;
}
