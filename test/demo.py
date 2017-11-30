#!/usr/bin/python3
import sys
import numpy as np
import random
import subprocess

import matplotlib
matplotlib.use('Qt5Agg')
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
import matplotlib.pyplot as plt

from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

class PlotCanvas(FigureCanvas):
    def __init__(self, parent, title, xlabel, ylabel, width=4, height=3, dpi=100):
        fig = Figure(figsize=(width, height), dpi=dpi)
        fig.subplots_adjust(left=0.15, bottom=0.15)
        self.axes = fig.add_subplot(111)
        self.axes.hold(False)

        FigureCanvas.__init__(self, fig)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(self,
                                   QSizePolicy.Expanding,
                                   QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)

        self.title = title
        self.xlabel = xlabel
        self.ylabel = ylabel
        fig.suptitle(title)

        self.reset()

    def reset(self):
        self.x = []
        self.y = []
        self.nexter = 0
        self.axes.axis([0, 5.4, 0, 1.1], size='xx-small')
        self.axes.set_xlabel(self.xlabel, size='small')
        self.axes.set_ylabel(self.ylabel, size='small')
        self.draw()

    def update_figure(self, step, y, xlimit = 10, ymin = 0, ylimit = 1200):
        self.nexter += step * 0.001
        self.x.append(self.nexter)
        self.y.append(y)
        self.axes.plot(self.x, self.y)
        self.axes.axis([0, xlimit, ymin, ylimit], size = 'xx-small')
        self.axes.set_xlabel(self.xlabel, size='small')
        self.axes.set_ylabel(self.ylabel, size='small')
        #self.figure.suptitle('a')
        self.draw()

class Demo(QWidget):
    def __init__(self):
        super().__init__()

        self._title = 'SCMKV Minitor'

        # line edit
        self.Lobjsizes = QLabel(self)
        self.Lobjsizes.move(50, 0)
        #self.Lobjsizes.setText('键值对大小(B)')
        self.Lobjsizes.setText('运行时间(ms)')
        self.Eobjsizes = QLineEdit(self)
        self.Eobjsizes.setText('0.0')
        self.Eobjsizes.move(50, 20)

        self.Lobjnum = QLabel(self)
        self.Lobjnum.move(200, 0)
        self.Lobjnum.setText('键值对数量(Million)')
        self.Eobjnum = QLineEdit(self)
        self.Eobjnum.setText('4')
        self.Eobjnum.move(200, 20)

        # progress bar
        self.pblabel = QLabel(self)
        self.pblabel.move(400, 0)
        self.pblabel.setText('进度条')
        self.pbar = QProgressBar(self)
        self.pbar.move(400, 20)

        bRun = QPushButton('Run', self)
        bRun.clicked.connect(self.changeWindowTitle)
        bRun.move(600, 20)

        self.img = PlotCanvas(self, 'objects inserted', 'time (s)', 'number (million)')
        self.img.move(50, 100)


        #self.img2 = PlotCanvas(self)
        self.img2 = PlotCanvas(self, 'throughput', 'time (s)', 'Mops')
        self.img2.move(450, 100)

        # main window
        self.setWindowTitle(self._title)
        self.setGeometry(300, 300, 900, 700)
        self.show()

    def changeWindowTitle(self, v):
        self.pbar.setValue(0)
        self.img.reset()
        self.img2.reset()
        self.objnum = int(self.Eobjnum.text())
        command = ['/home/zj/.git/scmkv/test/db-test']
        command.append('-c {}'.format(self.objnum))
        self.setWindowTitle(str(command))
        self.db = subprocess.Popen(command, stdout = subprocess.PIPE)
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_processbar)
        self.timer.start(1)
        self.ms = 0;
        self.step = 30

    def update_processbar(self):
        line = self.db.stdout.readline().strip()
        if line.isdigit():
            num = int(line) / 1000000
            self.pbar.setValue(num / self.objnum * 100)
            self.img.update_figure(self.step, num, xlimit = self.objnum / 2 + 0.5, ymin = 0, ylimit = self.objnum * 1.100)
            self.ms += self.step
            self.Eobjsizes.setText('{}'.format(self.ms))
            self.img2.update_figure(self.step, num * 1000 / self.ms, xlimit = self.objnum / 2 + 0.5, ymin = 0, ylimit = 3)
        else:
            self.pbar.setValue(100)
            self.timer.stop()



if __name__ == '__main__':
    app = QApplication(sys.argv)
    demo = Demo()
    sys.exit(app.exec_())

