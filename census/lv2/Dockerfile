# LV2 census image: Ubuntu 24.04, lilv, the LV2 plugin packages of the archive,
# and malloc-guard copied from the benchmark image (same Ubuntu release).
# Build: docker build -t rt-swe-bench/lv2:noble census/lv2/
FROM rt-swe-bench/base:jazzy-d71703b AS mg

FROM ubuntu:24.04@sha256:fe81560990fae86dc4743202b2cb4d3df98d0bcf949173cecef15ce1127ef852
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      g++ pkg-config binutils python3 liblilv-dev lilv-utils lv2-dev \
      calf-plugins x42-plugins lsp-plugins-lv2 swh-lv2 mda-lv2 zam-plugins fomp \
      invada-studio-plugins-lv2 eq10q guitarix-lv2 abgate ir.lv2 dragonfly-reverb-lv2 \
      avldrums.lv2 setbfree drumgizmo ste-plugins geonkick zynaddsubfx-lv2 \
 && rm -rf /var/lib/apt/lists/*
COPY --from=mg /opt/malloc_guard /opt/malloc_guard
ENV LD_LIBRARY_PATH=/opt/malloc_guard/lib
RUN dpkg-query -W -f='${Package}=${Version}\n' > /opt/apt-manifest.txt
