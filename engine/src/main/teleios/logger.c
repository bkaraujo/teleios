#include "teleios/teleios.h"
#include <string.h>

static TLLogLevel m_level = TL_LOG_LEVEL_VERBOSE;
void tl_logger_loglevel(const TLLogLevel desired){
    m_level = desired;
}

static const char *strings[] = {"VERBOSE", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
static const char *colors[] = { "\033[1;37m" , "\033[1;36m", "\033[1;34m", "\033[1;32m", "\033[1;33m", "\033[1;31m", "\033[1;31m" };

void tl_logger_write(const TLLogLevel level, const char *filename, const u32 lineno, const char *message, ...) {
    // Early return ~1 ns	✅ Perfeito
    if (level < m_level) { return; }

    char output[2048];
    // vsnprintf() ~100-500 ns	✅ Necessário
    va_list arg_ptr; va_start(arg_ptr, message);
    vsnprintf(output, sizeof(output), message, arg_ptr);
    va_end(arg_ptr);

    const char* slash = strrchr(filename, '/');         // strrchr Otimizado (SIMD)
    const char* backslash = strrchr(filename, '\\');    // strrchr Otimizado (SIMD)
    
    // Operador ternário ~2 ns	✅ Excelente
    const char* basename = (slash > backslash) ? slash + 1 : (backslash ? backslash + 1 : filename);
    
    // tl_time_clock() ~500-1000 ns	⚠️ Syscall
    // Impacto: ~50-60% do tempo total de logging
    // Syscall para obter timestamp do sistema
    // É o principal gargalo agora
    static TL_THREADLOCAL TLDateTime clock;
    tl_time_clock(&clock);
    
    char buffer[256];
    // snprintf() ~150-300 ns	✅ Bom
    int len = snprintf(buffer, sizeof(buffer), "%s%d-%02d-%02d %02d:%02d:%02d,%06u %10llu %-20s:%04d %-7s %s\n\033[1;30m",
        colors[level],
        clock.year, clock.month, clock.day,
        clock.hour, clock.minute, clock.second, clock.millis,
        tl_thread_id(),
        basename, 
        lineno, 
        strings[level], 
        output
    );
    
    // fwrite() ~50-100 ns	✅ Ótimo
    if (len > 0) fwrite(buffer, 1, len, stdout);
}