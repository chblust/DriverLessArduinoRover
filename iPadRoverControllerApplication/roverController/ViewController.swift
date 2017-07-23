//
//  ViewController.swift
//  roverController
//
//  Created by Chris Blust on 6/30/17.
//  Copyright © 2017 Chris Blust. All rights reserved.
//  IntelHacks 2017 Judges granted any rights required for judging
//

import UIKit
import CoreBluetooth
class ViewController: UIViewController, CBCentralManagerDelegate,
CBPeripheralDelegate {
    
    @IBOutlet weak var throttleSwitch: UISwitch!
    var manager:CBCentralManager!
    var roverPeripheral:CBPeripheral?
    var throttleCharacteristic: CBCharacteristic!
    let UUID = CBUUID(string: "19B10010-E8F2-537E-4F6C-D104768A1214")
    var currentSpeed = 0
    override func viewDidLoad() {
        super.viewDidLoad()
        throttleSwitch.setOn(false, animated: false)
        throttleSwitch.isHidden = true
        manager = CBCentralManager(delegate: self, queue: nil)
        
    }
    
    @IBAction func throttleSwitchValueChanged(_ sender: UISwitch) {
        let switchValue = sender.isOn
        var value: NSInteger = 0
        if switchValue{
            value = 1
        }
        roverPeripheral?.writeValue(NSData(bytes: &value, length: 1) as Data, for: throttleCharacteristic, type: .withResponse)
        
    }
    
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        
    }
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == CBManagerState.poweredOn {
            central.scanForPeripherals(withServices: nil, options: nil)
        } else {
            print("Bluetooth not available.")
        }
    }
    
    func centralManager(_ central: CBCentralManager,
                        didDiscover peripheral: CBPeripheral,
                        advertisementData: [String : Any],
                        rssi RSSI: NSNumber){
        print("Found peripheral")
        
        
        self.roverPeripheral = peripheral
        self.roverPeripheral?.delegate = self
        self.manager.stopScan()
        manager.connect(self.roverPeripheral!, options: nil)
        
    }
    
    func centralManager(_
        central: CBCentralManager,
                        didConnect peripheral: CBPeripheral) {
        print("Connected!")
        peripheral.discoverServices(nil)
    }
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        print(error)
    }
    
    func peripheral(
        _ peripheral: CBPeripheral,
        didDiscoverServices error: Error?) {
        for service in peripheral.services! {
            let thisService = service as CBService
            print(service)
            
            
            peripheral.discoverCharacteristics(
                nil,
                for: thisService
            )
            
            
        }
    }
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        for characteristic in service.characteristics!{
            print(characteristic.uuid)
            if characteristic.uuid == UUID{
                self.throttleCharacteristic = characteristic
                self.throttleSwitch.isHidden = false
                print("Found Characteristic")
            }
        }
    
}

}
