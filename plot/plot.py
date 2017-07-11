import matplotlib.pyplot as plt
import pandas as pds

GCC_INTEGER_TYPES = {
    '1': 'uint08_t',
    '2': 'uint16_t',
    '4': 'uint32_t',
    '8': 'uint64_t'
}

CACHE_SIZES_IN_KB = {
    'L1': 32,
    'L2': 256,
    'L3': 24576
}

converters = {'Integer_Type': lambda x: GCC_INTEGER_TYPES[x]}

data_prefetch = pds.read_csv('./results/prefetch.csv', ';', skipinitialspace=True, converters=converters)
data_no_prefetch = pds.read_csv('./results/non-prefetch.csv', ';', skipinitialspace=True, converters=converters)


def plot_line(data, options):
    # Setup plot and data.
    fig, ax = plt.subplots(1, 1)
    data.groupby(options['groupby']).plot(x=options['x_col'], y=options['y_col'], ax=ax, logx=True)

    # Remove the plot frame lines.
    ax.spines['top'].set_visible(False)
    ax.spines['bottom'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['left'].set_visible(False)

    # Remove the tick marks and use a grid instead.
    plt.tick_params(axis='both', which='both', bottom='off', top='off',
                    labelbottom='on', left='off', right='off', labelleft='on')

    # Show cache sizes of L1, L2 and L3.
    for cache in CACHE_SIZES_IN_KB:
        plt.axvline(CACHE_SIZES_IN_KB[cache], ls='--', lw=.5, c='k', alpha=.3)
        plt.text(CACHE_SIZES_IN_KB[cache] * 1.2, data[[options['y_col']]].max(), cache, color='k', alpha=.3)

    # Set texts.
    plt.title(options['title'], y=1.10)
    plt.xlabel(options['x_label'])
    plt.ylabel(options['y_label'])
    plt.legend([v[0] for v in data.groupby(options['groupby'])])
    plt.figtext(0, -0.1, options['subtitle'], fontsize=8)

    # Export Plot
    plt.savefig('./plot/' + options['out_file'] + '.pdf', bbox_inches='tight', dpi=200)
    plt.savefig('./plot/' + options['out_file'] + '.png', bbox_inches='tight', dpi=200)
    # plt.show()


def base_benchmark():
    # Aggregate Benchmark
    df_1 = data_prefetch.query('Benchmark_Type == 0').query('Thread_Count == 1')
    options = {
        'title': 'Aggregate Bandwidth as a Function of the Column Size and the Bitcase',
        'subtitle': 'Intel(R) Xeon(R) X7560 @ 2.27GHz, w/ Prefetcher, 1 Thread',
        'x_col': 'Vector_Size',
        'x_label': 'Attribute Vector Size (in Byte)',
        'y_col': 'Bandwidth',
        'y_label': 'Effective Bandwidth (in GB/s)',
        'groupby': 'Integer_Type',
        'out_file': '1_aggregate_w-pref_1-thread'
    }
    plot_line(df_1, options)

    # Scan Benchmark
    df_2 = data_prefetch.query('Benchmark_Type == 1').query('Thread_Count == 1')
    options['title'] = 'Column Scan Bandwidth as a Function of the Column Size and the Bitcase'
    options['subtitle'] = 'Intel(R) Xeon(R) X7560 @ 2.27GHz, w/ Prefetcher, 1 Thread'
    options['out_file'] = '1_scan_w-pref_1-thread'
    plot_line(df_2, options)


def prefetch_benchmark():
    # Aggregate w/o Prefetch
    df_1 = data_no_prefetch.query('Benchmark_Type == 0').query('Thread_Count == 1')
    options = {
        'title': 'Aggregate Bandwidth as a Function of the Column Size and the Bitcase',
        'subtitle': 'Intel(R) Xeon(R) X7560 @ 2.27GHz, w/o Prefetcher, 1 Thread',
        'x_col': 'Vector_Size',
        'x_label': 'Attribute Vector Size (in Byte)',
        'y_col': 'Bandwidth',
        'y_label': 'Effective Bandwidth (in GB/s)',
        'groupby': 'Integer_Type',
        'out_file': '2_aggregate_wo-pref_1-thread'
    }
    plot_line(df_1, options)


def multi_thread_benchmark():
    # Multi-Threading Benchmark
    df_1 = data_prefetch.query('Benchmark_Type == 0').query('Integer_Type == "uint64_t"')
    options = {
        'title': 'Aggregate Bandwidth as Function of Column Size and Thread Count',
        'subtitle': 'Intel(R) Xeon(R) X7560 @ 2.27GHz, w/ Prefetcher, uint64_t',
        'x_col': 'Vector_Size',
        'x_label': 'Attribute Vector Size (in Byte)',
        'y_col': 'Bandwidth',
        'y_label': 'Effective Bandwidth (in GB/s)',
        'groupby': 'Thread_Count',
        'out_file': '3_aggregate_w-pref_multi-thread'
    }
    plot_line(df_1, options)

    # Multi-Threading w/o Prefetcher
    df_2 = data_no_prefetch.query('Benchmark_Type == 0').query('Integer_Type == "uint64_t"')
    options['subtitle'] = 'Intel(R) Xeon(R) X7560 @ 2.27GHz, w/o Prefetcher, uint64_t'
    options['out_file'] = '3_aggregate_wo-pref_multi-thread'
    plot_line(df_2, options)

    # Multi-Threading w Prefetcher
    df_3 = data_prefetch.query('Benchmark_Type == 1').query('Integer_Type == "uint64_t"')
    options['subtitle'] = 'Intel(R) Xeon(R) X7560 @ 2.27GHz, w Prefetcher, uint64_t'
    options['out_file'] = '3_scan_w-pref_multi-thread'
    options['title'] = 'Column Scan Bandwidth as a Function of the Column Size and the Bitcase'
    plot_line(df_3, options)


base_benchmark()
prefetch_benchmark()
multi_thread_benchmark()
