#!/usr/bin/env python2
# -*- coding: utf-8 -*-
import logging
import sys

from PyQt5.QtWidgets import QApplication, QWidget,  QVBoxLayout, QTableView
from PyQt5.QtCore import QAbstractTableModel, Qt, QThread

from pymodbus.datastore import ModbusSequentialDataBlock
from pymodbus.datastore import ModbusSlaveContext, ModbusServerContext
from pymodbus.server.sync import ModbusTcpServer, ModbusSerialServer
from pymodbus.transaction import ModbusSocketFramer as MSF
from pymodbus.transaction import ModbusRtuFramer as MRF

usage = '''modbussim.py <ip> <port> <slave> <size>                      -- TCP
modbussim.py <port> <slave> <size> <baud> <pairty> <timeout> -- RTU
'''


class MBSlave(ModbusSlaveContext):

    def __init__(self, co, di, hr, ir):
        super(MBSlave, self).__init__(co=co, di=di, hr=hr, ir=ir)

    def setModel(self, model):
        self.model = model

    def setValues(self, fx, address, values):
        print('slave:', fx, address, values)
        super(MBSlave, self).setValues(fx, address, values)
        if fx == 5:
            self.model.data[0][address] = (values[0] and 1 or 0)
        elif fx == 6:
            self.model.data[2][address] = values[0]


class MBTCPServer(QThread):

    def __init__(self, ip, port, unit, data, parent):
        super(MBTCPServer, self).__init__(parent)
        self.slave = MBSlave(ModbusSequentialDataBlock(1, data[0]),
                             ModbusSequentialDataBlock(1, data[1]),
                             ModbusSequentialDataBlock(1, data[2]),
                             ModbusSequentialDataBlock(1, data[3]))
        self.ctx = ModbusServerContext(slaves={unit: self.slave}, single=False)
        self.server = ModbusTcpServer(self.ctx, framer=MSF, address=(ip, port))

    def run(self):
        self.server.serve_forever()

    def quit(self):
        self.server.server_close()
        self.server.shutdown()
        super(MBTCPServer, self).quit()


class MBSerialServer(QThread):

    def __init__(self, port, unit, data, baud, par, sec, parent):
        super(MBSerialServer, self).__init__(parent)
        self.slave = MBSlave(ModbusSequentialDataBlock(1, data[0]),
                             ModbusSequentialDataBlock(1, data[1]),
                             ModbusSequentialDataBlock(1, data[2]),
                             ModbusSequentialDataBlock(1, data[3]))
        self.ctx = ModbusServerContext(slaves={unit: self.slave}, single=False)
        sb = par == 'N' and 2 or 1
        self.server = ModbusSerialServer(self.ctx, framer=MRF, port=port,
                                         stopbits=sb, bytesize=8, parity=par,
                                         baudrate=baud, timeout=sec)
        print(self.server)

    def run(self):
        self.server.serve_forever()

    def quit(self):
        self.server.server_close()
        super(MBSerialServer, self).quit()


class MBModel(QAbstractTableModel):

    headers = ['coil', 'discrete', 'holding', 'input']

    def __init__(self, data, parent):
        super(MBModel, self).__init__(parent)
        self.data = data

    def columnCount(self, parent):
        return len(self.headers)

    def data(self, index, role):
        if role == Qt.DisplayRole or role == Qt.EditRole:
            return self.data[index.column()][index.row()]
        elif role == Qt.ToolTipRole:
            a = '{:016b}'.format(self.data[index.column()][index.row()])
            return ' '.join([a[i:i+4] for i in range(0, len(a), 4)])

    def flags(self, index):
        return (Qt.ItemIsSelectable | Qt.ItemIsEditable |
                Qt.ItemIsEnabled | Qt.ToolTip)

    def headerData(self, sec, orientation, role):
        if role == Qt.DisplayRole:
            return orientation == Qt.Horizontal and self.headers[sec] or sec+1

    def rowCount(self, parent):
        return len(self.data[0])

    def setData(self, index, value, role):
        if role == Qt.EditRole:
            v = value
            if index.column() == 0 or index.column() == 1:
                v = v and 1 or 0
            if 0 <= v <= 0xffff:
                self.data[index.column()][index.row()] = v
                self.slave.setValues(index.column() + 1, index.row(), [v])
                return True
        return False

    def setSlave(self, slave):
        self.slave = slave


class MBWindow(QWidget):

    def __init__(self, data):
        super(MBWindow, self).__init__()
        vbox = QVBoxLayout(self)
        self.tm = MBModel(data, self)
        self.tv = QTableView(self)
        self.tv.setModel(self.tm)

        self.setGeometry(300, 300, 500, 500)
        self.setWindowTitle('Modbus Simulator')
        vbox.addWidget(self.tv)
        self.setLayout(vbox)


def main():
    if len(sys.argv) < 4:
        print(usage)
        return
    elif sys.argv[1][0].isdigit():
        proto, port, unit = 'TCP', int(sys.argv[2]), int(sys.argv[3])
        num = int(sys.argv[4])
    else:
        proto, port, unit = 'RTU', sys.argv[1], int(sys.argv[2])
        num, baud = int(sys.argv[3]), int(sys.argv[4])
        parity, timeout = sys.argv[5], int(sys.argv[6])
    app = QApplication(sys.argv)
    data = [[0] * num, [0] * num, [0] * num, [0] * num]
    w = MBWindow(data)
    if proto == 'TCP':
        server = MBTCPServer(sys.argv[1], port, unit, data, w)
    else:
        server = MBSerialServer(port, unit, data, baud, parity, timeout, w)
    w.tm.setSlave(server.slave)
    server.slave.setModel(w.tm)
    server.start()
    w.show()
    app.exec_()
    server.quit()


if __name__ == '__main__':
    logging.basicConfig()
    logging.getLogger().setLevel(logging.DEBUG)
    main()
