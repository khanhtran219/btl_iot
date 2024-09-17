import os
import tensorflow as tf
import cv2

# Optional: Disable oneDNN optimizations if needed
os.environ['TF_ENABLE_ONEDNN_OPTS'] = '0'

def prediction(img, model, class_names):
    # Rescaling image
    img = img / 255.0

    # Converting to tensor
    tensor_img = tf.convert_to_tensor(img, dtype=tf.float32)
    tensor_img = tf.image.resize(tensor_img, [224, 224])
    tensor_img = tensor_img[tf.newaxis, ...]
    tensor = model(tensor_img)['dense_3']
    tmp = tensor.numpy()
    class_index = tmp.argmax()

    return class_names[class_index]

if __name__ == '__main__':
    # Loading model
    model_path = 'Trash_classifier-main\saved_models\MobileNetV2'
    loaded_model = tf.saved_model.load(model_path)
    model = loaded_model.signatures['serving_default']

    # Class names
    class_names = ['cardboard','metal','paper','plastic','trash','glass','organic']


    # Start video capture from the webcam
    cap = cv2.VideoCapture(0)  # 0 is the default camera

    if not cap.isOpened():
        print("Error: Could not open webcam.")
        exit()

    while True:
        # Capture frame-by-frame
        ret, frame = cap.read()

        if not ret:
            print("Error: Could not read frame.")
            break

        # Convert the frame to RGB (default color format for TensorFlow models)
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        # Perform prediction
        predicted_class = prediction(rgb_frame, model, class_names)

        # Display the resulting frame with prediction
        cv2.putText(frame, f'Prediction: {predicted_class}', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2, cv2.LINE_AA)
        cv2.imshow('Webcam Feed', frame)

        # Press 'q' to exit the loop
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # When everything done, release the capture
    cap.release()
    cv2.destroyAllWindows()
