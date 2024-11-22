package src;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

public class Flight {
    public Integer flightId;
    public String sourcePlace;
    public String destinationPlace;
    public float airfare;
    public int seatAvailability;

    // 构造函数
    public Flight(Integer flightId, String sourcePlace, String destinationPlace,
                  float airfare, int seatAvailability) {
        this.flightId = (flightId != null && flightId > 0) ? flightId : 0;

        // 如果出发地为空，则使用默认值
        this.sourcePlace = (sourcePlace != null && !sourcePlace.isEmpty()) ? sourcePlace : "Unknown";

        // 如果目的地为空，则使用默认值
        this.destinationPlace = (destinationPlace != null && !destinationPlace.isEmpty()) ? destinationPlace : "Unknown";

        // 如果票价为0或负数，则设置为默认值
        this.airfare = (airfare > 0) ? airfare : 100.0f;

        // 如果座位数为0或负数，则设置为默认值
        this.seatAvailability = (seatAvailability > 0) ? seatAvailability : 50;
    }

    // 序列化 Flight 对象，将航班信息转换为字节数组
    public byte[] marshal() {
        byte[] sourceBytes = sourcePlace.getBytes(StandardCharsets.UTF_8);
        byte[] destinationBytes = destinationPlace.getBytes(StandardCharsets.UTF_8);

        ByteBuffer buffer = ByteBuffer.allocate(4 + 4 + sourceBytes.length + 4 + destinationBytes.length + 4 + 4);

        buffer.putInt(flightId);                                    // 4 bytes 航班ID
        buffer.putInt(sourceBytes.length);                          // 4 bytes 出发地长度
        buffer.put(sourceBytes);                                    // N bytes 出发地
        buffer.putInt(destinationBytes.length);                     // 4 bytes 目的地长度
        buffer.put(destinationBytes);                               // N bytes 目的地
        buffer.putFloat(airfare);                                   // 4 bytes 票价
        buffer.putInt(seatAvailability);                            // 4 bytes 座位可用数量

        return buffer.array();  // 返回字节数组
    }

    // 反序列化，将字节数组转换为 Flight 对象
    public static Flight unmarshal(byte[] bytes) {
        ByteBuffer buffer = ByteBuffer.wrap(bytes);

        int flightId = buffer.getInt();
        int sourceLength = buffer.getInt();
        byte[] sourceBytes = new byte[sourceLength];
        buffer.get(sourceBytes);
        String sourcePlace = new String(sourceBytes, StandardCharsets.UTF_8);

        int destinationLength = buffer.getInt();
        byte[] destinationBytes = new byte[destinationLength];
        buffer.get(destinationBytes);
        String destinationPlace = new String(destinationBytes, StandardCharsets.UTF_8);

        float airfare = buffer.getFloat();
        int seatAvailability = buffer.getInt();

        return new Flight(flightId, sourcePlace, destinationPlace, airfare, seatAvailability);
    }

    @Override
    public String toString() {
        return "Flight ID: " + flightId +
                ", Source: " + sourcePlace +
                ", Destination: " + destinationPlace +
                ", Airfare: " + airfare +
                ", Seat Availability: " + seatAvailability;
    }
}
