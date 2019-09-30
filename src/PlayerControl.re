[@bs.module "./images/play.svg"] external play: string = "default";
[@bs.module "./images/pause.svg"] external pause: string = "default";

[@react.component]
let make = (~isPlaying, ~handleClick) => {
  <div className="w3 mh3">
    <button
      className="bn outline-0 pointer bg-transparent" onClick=handleClick>
      {isPlaying ? <img src=pause alt="Pause" /> : <img src=play alt="Play" />}
    </button>
  </div>;
};