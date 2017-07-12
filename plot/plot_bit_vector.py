import pandas as pds

# Seaborn wraps matplotlib for ease of use
# See http://seaborn.pydata.org/api.html
import seaborn as sns

# rc matplotlib configuration dictionary
# https://stackoverflow.com/a/37405330
rc = {"savefig.dpi": 200}

# Set nice colors and spine/tick styles
# Optional: context="talk" to enlarge font for presentations
sns.set(style="white",
        palette="Paired",
        font='Ubuntu',
        context="talk",
        rc=rc)

##################################
# TABLE SCAN
##################################
data = pds.read_csv('./results/bit_vector_skylake.csv', ';', skipinitialspace=True)

type_order = ['Regular Vector', 'Bit-Packed', 'AVX']
operation_order = ['Point Lookups', 'Table Scan']

# Mind aspect ratio for 16:9 presentation
g = sns.factorplot(kind="bar",
                   data=data,
                   x="operation",
                   order=operation_order,
                   y="no_of_tuples",
                   hue="type",
                   hue_order=type_order,
                   legend=False,
                   legend_out=False,
                   aspect=1.77)

# Add custom legend to remove unnecessary heading
g.add_legend(label_order=type_order)

g.set(xlabel="", ylabel="Tuples per Second", ylim=[0, 700000000])

# Remove left/bottom borders
g.despine(left=True, bottom=True)

# TODO: Box grid like prettyplotlib
# We could also use g.fig to access the matplotlib object

g.savefig("./plot/04_bit_vector_skylake.pdf")
g.savefig("./plot/04_bit_vector_skylake.png")

##################################
# TABLE SCAN
##################################
data = pds.read_csv('./results/bit_vector_table_scan.csv', ';', skipinitialspace=True)

operation_order = ['Regular Vector', 'Bit-Packed', 'AVX']
cpu_order = ['Haswell', 'Broadwell', 'Skylake']

# Mind aspect ratio for 16:9 presentation
g = sns.factorplot(kind="bar",
                   data=data,
                   x="type",
                   order=operation_order,
                   y="no_of_tuples",
                   hue="cpu_generation",
                   hue_order=cpu_order,
                   legend=False,
                   legend_out=False,
                   aspect=1.77)

# Add custom legend to remove unnecessary heading
g.add_legend(label_order=cpu_order)

g.set(xlabel="", ylabel="Tuples per Second", ylim=[0, 700000000])

# Remove left/bottom borders
g.despine(left=True, bottom=True)

# TODO: Box grid like prettyplotlib
# We could also use g.fig to access the matplotlib object

g.savefig("./plot/04_bit_vector_table_scan.pdf")
g.savefig("./plot/04_bit_vector_table_scan.png")

##################################
# POINT LOOKUPS
##################################
data = pds.read_csv('./results/bit_vector_point_lookup.csv', ';', skipinitialspace=True)

operation_order = ['Bit-Packed', 'Bit-Packed w/ SIMD', 'Regular Vector', 'Regular Vector w/ SIMD']
cpu_order = ['Haswell', 'Broadwell', 'Skylake']
# Mind aspect ratio for 16:9 presentation
g = sns.factorplot(kind="bar",
                   data=data,
                   x="type",
                   order=operation_order,
                   y="no_of_tuples",
                   hue="cpu_generation",
                   hue_order=cpu_order,
                   legend=False,
                   legend_out=False,
                   aspect=1.77)

# Add custom legend to remove unnecessary heading
g.add_legend(label_order=cpu_order)

g.set(xlabel="", ylabel="Tuples per Second", ylim=[0, 700000000])

# Fix sizing of long labels
g.set_xticklabels(fontsize=9)

# Remove left/bottom borders
g.despine(left=True, bottom=True)

# TODO: Box grid like prettyplotlib
# We could also use g.fig to access the matplotlib object

g.savefig("./plot/04_bit_vector_point_lookups.pdf")
g.savefig("./plot/04_bit_vector_point_lookups.png")
