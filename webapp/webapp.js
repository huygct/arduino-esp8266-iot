angular.module('myApp', [
  'ngRoute',
  'mobile-angular-ui',
  'btford.socket-io'
]).config(function ($routeProvider) {
  $routeProvider.when('/', {
    templateUrl: 'home.html',
    controller: 'Home'
  });
}).factory('mySocket', function (socketFactory) {
  var myIoSocket = io.connect('/webapp');	//Tên namespace webapp

  mySocket = socketFactory({
    ioSocket: myIoSocket
  });
  return mySocket;

  /////////////////////// Những dòng code ở trên phần này là phần cài đặt, các bạn hãy đọc thêm về angularjs để hiểu, cái này không nhảy cóc được nha!
}).controller('Home', function ($scope, mySocket) {
  ////Khu 1 -- Khu cài đặt tham số 
  //cài đặt một số tham số test chơi
  //dùng để đặt các giá trị mặc định
  $scope.date = new Date().toISOString().slice(0,10);
  $scope.leds_status = [0];
  $scope.temp = {};
  $scope.motorThuan = false;
  $scope.motorNghich = false;
  $scope.fanStatus = false;
  $scope.AUTO = true;

  $scope.changeStatus = function () {
    console.log('---- ', $scope.AUTO)
    var json = {
      "status": $scope.AUTO ? 1 : 0 
    }
    mySocket.emit("STATUS", json)
  }
  ////Khu 2 -- Cài đặt các sự kiện khi tương tác với người dùng
  //các sự kiện ng-click, nhấn nút
  $scope.updateSensor = function () {
    mySocket.emit("RAIN")
  }

  $scope.changeLED = function () {
    var json = {
      "led": [$scope.leds_status[0] ? 1 : 0, $scope.leds_status[1] ? 1 : 0]
    }
    mySocket.emit("LED", json)
  }

  $scope.runMotor = function () {
    var json = {
      "role": $scope.motorThuan ? 1 : $scope.motorNghich ? 2 : 0
    }
    mySocket.emit("MOTOR", json)
  }

  $scope.runFan = function () {
    var json = {
      "role": $scope.fanStatus ? 1 : 0
    }
    mySocket.emit("FAN", json)
  }

  ////Khu 3 -- Nhận dữ liệu từ Arduno gửi lên (thông qua ESP8266 rồi socket server truyền tải!)
  //các sự kiện từ Arduino gửi lên (thông qua esp8266, thông qua server)
  mySocket.on('TEM_STATUS', function (json) {
    // console.log("Nhiet do ", json)
    $scope.temp = {
      t: json.t,
      h: json.h,
      l: json.l
    };
    $scope.AUTO = json.status === 1;
  })
  //Khi nhận được lệnh LED_STATUS
  mySocket.on('LED_STATUS', function (json) {
    //Nhận được thì in ra thôi hihi.
    // console.log("recv LED", json)
    $scope.leds_status = json.data
  })


  //// Khu 4 -- Những dòng code sẽ được thực thi khi kết nối với Arduino (thông qua socket server)
  mySocket.on('connect', function () {
    console.log("connected")
    // mySocket.emit("RAIN") //Cập nhập trạng thái mưa
  })

});