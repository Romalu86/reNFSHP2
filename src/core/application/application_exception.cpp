#include "nfs/core/application/application.hpp"
#include "nfs/core/errors/assertions.hpp"
#include <cstdio>
namespace nfs::application::pending {
namespace {
#include "application_exception_text.inc"
const char *exception_description = "";
} // namespace

int exception_filter(EXCEPTION_POINTERS *exception) {
    CONTEXT *context = exception->ContextRecord;
    const DWORD code = exception->ExceptionRecord->ExceptionCode;
    if (assertions::abort_only)
        return EXCEPTION_CONTINUE_SEARCH;
    if (code == 2147483651u) {
        ++context->Eip;
        exception_description = exception_breakpoint_text;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    char unknown[128];
    switch (code) {
    case 2147483650u:
        exception_description = exception_misalignment_text;
        break;
    case 2147483652u:
        exception_description = exception_single_step_text;
        break;
    case 3221225477u:
        exception_description = exception_access_violation_text;
        break;
    case 3221225509u:
        exception_description = exception_noncontinuable_text;
        break;
    case 3221225612u:
        exception_description = exception_array_bounds_text;
        break;
    case 3221225613u:
        exception_description = exception_float_denormal_operand_text;
        break;
    case 3221225614u:
        exception_description = exception_float_divide_by_zero_text;
        break;
    case 3221225615u:
        exception_description = exception_float_inexact_result_text;
        break;
    case 3221225616u:
        exception_description = exception_float_invalid_operation_text;
        break;
    case 3221225617u:
        exception_description = exception_float_overflow_text;
        break;
    case 3221225618u:
        exception_description = exception_float_stack_check_text;
        break;
    case 3221225619u:
        exception_description = exception_float_underflow_text;
        break;
    case 3221225620u:
        exception_description = exception_integer_divide_by_zero_text;
        break;
    case 3221225621u:
        exception_description = exception_integer_overflow_text;
        break;
    case 3221225622u:
        exception_description = exception_privileged_instruction_text;
        break;
    default:
        std::sprintf(unknown, exception_unknown_code_format, code);
        exception_description = unknown;
        break;
    }
    int written = std::sprintf(exception_text, exception_copy_hint_text);
    written += std::sprintf(exception_text + written, exception_description_format, exception_description);
    written += std::sprintf(exception_text + written, exception_version_format, 242);
    written += std::sprintf(exception_text + written, exception_instruction_pointer_format, context->Eip);
    written += std::sprintf(exception_text + written, exception_stack_registers_format, context->Esp, context->Ebp);
    written += std::sprintf(exception_text + written, exception_accumulator_register_format, context->Eax);
    written += std::sprintf(exception_text + written, exception_base_register_format, context->Ebx);
    written += std::sprintf(exception_text + written, exception_counter_register_format, context->Ecx);
    written += std::sprintf(exception_text + written, exception_data_register_format, context->Edx);
    std::sprintf(exception_text + written, exception_index_registers_format, context->Esi, context->Edi);
    return EXCEPTION_EXECUTE_HANDLER;
}
} // namespace nfs::application::pending
