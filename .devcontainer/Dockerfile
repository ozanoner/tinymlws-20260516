FROM espressif/idf:v5.5.4

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

RUN apt-get update && apt-get install -y udev clang-format vim-tiny ffmpeg bsdmainutils
RUN curl -L https://wokwi.com/ci/install.sh | sh
RUN curl -LsSf https://astral.sh/uv/install.sh | sh

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc
RUN echo "set -o vi" >> ~/.bashrc

RUN git config --global core.editor vi

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

CMD ["/bin/bash"]
